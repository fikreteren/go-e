#include "Process.hpp"

std::atomic<bool> Process::_force_kill{false};

Process::Process()
{
    std::string msg = fmt::format("PID:{} spawned by Parent-PID:{} successfully.", getpid(), getppid());
    printMessage(MsgTypes::INFO, msg);

    if (mapToPhyPage())
    {
        std::exit(-1); // kill the process, if it could not map his vpage.
    }

    _eva_thread = std::thread(&Process::evaluateTermination, this);
    _work_thread = std::thread (&Process::interCommunicator, this);
}

Process::~Process()
{
    if (_eva_thread.joinable()) 
    {
        _eva_thread.join();
    } 
    if (_work_thread.joinable()) 
    {
        _work_thread.join();
    }

    if (_ipc_buffer) 
    {
        munmap(_ipc_buffer, SHM_BlockSize);
        _ipc_buffer = nullptr;
    }
    if (_ipc_buffer_lock) 
    {
        munmap(_ipc_buffer_lock, sizeof(sem_t));
        _ipc_buffer_lock = nullptr;
    }
}

void Process::sleepUntilTermination(void)
{
    while (!_force_kill.load(std::memory_order_acquire))
    {
        _force_kill.wait(false);
    }
}   

Process& Process::getInstance() 
{
    static Process instance;
    return instance;
}

int Process::mapToPhyPage(void)
{
    // First, we have to create the shared memory objects and allocate memory for them.
    int fd_buf = shm_open(SHM_BlockIdentifier, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    // We have to detect first creation to not re-init the semaphore, which leads
    // to undefined behaviour according to the POSIX standard.
    int fd_buf_lock = shm_open(SHM_MemProt, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
    bool already_initialized = true;
    if (fd_buf_lock == -1 && errno == EEXIST)
    {
        already_initialized = false;
        fd_buf_lock = shm_open(SHM_MemProt, O_RDWR, S_IRUSR | S_IWUSR);
    }

    if (fd_buf == -1 || fd_buf_lock == -1)
    {
        std::string error_msg = fmt::format("Failed to open SHM descriptors for PID:{}!", getpid());
        printMessage(MsgTypes::ERROR, error_msg);
        return -1;
    }

    // calling ftruncate more than once with unchanged length will not affect/change
    // the shared memory object.
    if (ftruncate(fd_buf, SHM_BlockSize) == -1 || ftruncate(fd_buf_lock, sizeof(sem_t)))    
    {
        std::string error_msg = fmt::format("Failed to truncate the opened SHM objects for PID:{}", getpid());
        printMessage(MsgTypes::ERROR, error_msg);
        close(fd_buf);
        close(fd_buf_lock);

        return -1;       
    }

    // Afterwards, this objects have to be mapped in a the virtual adress space.
    void* addr_buf = mmap(nullptr, SHM_BlockSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_buf, 0);
    void* addr_buf_lock = mmap(nullptr, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd_buf_lock, 0);

    if (addr_buf == MAP_FAILED || addr_buf_lock == MAP_FAILED)
    {
        std::string error_msg = fmt::format("Failed to map vpage to ppage for PID:{}", getpid());
        printMessage(MsgTypes::ERROR, error_msg);
        close(fd_buf);
        close(fd_buf_lock);
        munmap(addr_buf, SHM_BlockSize);
        munmap(addr_buf_lock, sizeof(sem_t));
        return -1;       
    }   
    
    // It has been decided to use a sem_t, because it is easier to work with 
    // compared to e.g. POSIX pthread mutexes. By easier I mean, that no addtional, 
    // attr. setup-routine is needed. Of course, constructs like boost::interprocess, 
    // would be possible to use too. 
    _ipc_buffer_lock = static_cast<sem_t*>(addr_buf_lock);
    _ipc_buffer = static_cast<IPCDatagramm*>(addr_buf);

    // mappings remain valid, we can clsoe the file descriptors, because
    // they are not needed anymore.
    close(fd_buf);
    close(fd_buf_lock);

    if (already_initialized) 
    {
        std::memset(_ipc_buffer_lock, 0, sizeof(sem_t));
        if (sem_init(_ipc_buffer_lock, 1 /*pshared*/, 1 /*initial count*/) == -1) 
        {
            std::string error_msg = fmt::format("Initializing Semaphore failed for PID:{}", getpid());
            printMessage(MsgTypes::ERROR, error_msg); 

            munmap(_ipc_buffer, SHM_BlockSize);
            munmap(_ipc_buffer_lock, sizeof(sem_t));
            _ipc_buffer = nullptr;
            _ipc_buffer_lock = nullptr;
            return -1;
        }
    }

    return 0;
}

void Process::evaluateTermination(void)
{
    while (!_force_kill.load(std::memory_order_acquire))
    {       
        size_t x = getRandValue(0, std::numeric_limits<size_t>::max());
        size_t y = getRandValue(1, 100);
        
        if ((x % y) == 0)  // just some arbitrary choosen death condition
        {
            std::string msg = fmt::format("PID:{} has been choosen for termination.", getpid());
            printMessage(MsgTypes::DEATH, msg); 

            _force_kill.store(true, std::memory_order_release); 
            break; // exit this thread
        }
        sleep(1); // not neccessary to evaluate process status that often
    }
}

size_t Process::getRandValue(size_t min, size_t max)
{
    thread_local std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<size_t> rand_val(min, max);
    
    return rand_val(generator);
}

void Process::setMsgForIPC(IPCDatagramm* const packet)
{
    std::lock_guard<std::mutex> lock(_ipc_msg_lock);
    size_t rand_entry = getRandValue(0, example_data.size()-1);
    const std::string& entry = example_data.at(rand_entry);
    memcpy(packet->_data, entry.c_str(), entry.size()+1);

    _ipc_msg = packet;
}

void Process::interCommunicator()
{
    while (!_force_kill.load(std::memory_order_acquire))
    {
        sem_wait(_ipc_buffer_lock);
        
        // check if a termination request has been ordered.
        if (_force_kill.load(std::memory_order_relaxed)) 
        {
            // release the lock and exit the thread.
            sem_post(_ipc_buffer_lock);
            break;
        }

        // read and print the data in the SHM space.
        IPCDatagramm* ipc_package = static_cast<IPCDatagramm*>(_ipc_buffer);

        if (ipc_package && ipc_package->_not_dirty) // Only read the data once between processes.
        {
            std::string msg = fmt::format("PID:{} has read following content:     Package-ID:{}->{}", 
                getpid(), ipc_package->_pckg_id, ipc_package->_data);
        
            printMessage(MsgTypes::IPC, msg);
            
            ipc_package->_not_dirty = false;
        }

        // write something new to it after reading it.
        _ipc_msg_lock.lock();
        std::memcpy(_ipc_buffer, _ipc_msg, sizeof(IPCDatagramm));
        _ipc_msg_lock.unlock();

        sem_post(_ipc_buffer_lock);
        sleep(2);
    }
    _force_kill.notify_all(); // Notify the "main thread" to wake up and exit the process
}

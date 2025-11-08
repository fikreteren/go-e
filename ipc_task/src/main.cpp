#include "util.hpp"
#include "Process.hpp"

/**
 * @brief   Reading the number of additional processes from stdin, 
 *          which will be created via the fork syscall.
 * 
 * @return  The number of user-requested processes between 2 and 10.
 */
size_t getNrProcessByUsr(void) 
{
    size_t nr_process = 0; 

    while (true) 
    {
        std::cout << "Enter the number of processes to be spawned (2-10): ";
        
        // A upper boundary of user-definable number of processes,
        // will be neccesary, as forking new processes will be expensive 
        // in terms of memory (even with CoW) and performance (flushing TLB cache when 
        // new process memory space is loaded).
        if ((std::cin >> nr_process) && (nr_process >= 2 && nr_process <= 10))
        {
            break;
        }
        else
        {
            printMessage(MsgTypes::ERROR, "Invalid input. Please try again.");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore all characters till newline comes
        }
    }
    return nr_process;
}

/**
 * @brief   This blocking function, will be called by the parent process
 *          ensuring that all created child's terminate.
 * 
 * @param   pids A reference to the vector structure, that holds all child PIDs.
 * @return  none
 */
void waitForChildsTermination(std::vector<pid_t>& pids)
{
    for (auto p : pids)
    {
        waitpid(p, nullptr, 0);
    }
}

/**
 * @brief   This function will fork <nr_process> child processes, where each
 *          child process will join the shared memory structure and sent a 
 *          package. Afterwards, the child process will wait until exit. 
 *          The parent process will save all child PIDs in the referenced
 *          vector structure.
 * 
 * @param   nr_process The number of user-requested processes.
 * @param   pids A reference to the vector structure, that holds all child PIDs.
 * @return  The error code, where 0 means success, and other than that meaning failure.
 */
int createProcess(size_t nr_process, std::vector<pid_t>& pids)
{
    for (size_t iter = 0; iter < nr_process; ++iter)
    {
        pid_t pid = fork(); 

        if (pid == -1)
        {
            printMessage(MsgTypes::ERROR, "Forking a new child proccess failed!");
            waitForChildsTermination(pids);
            return -1;
        }
        else if (pid == 0) // child process
        {
            IPCDatagramm _packet{
                getpid(),  
                true, // checks if the package is dirty (=already opened).
                "Default"
            };

            Process::getInstance().setMsgForIPC(&_packet);
            Process::getInstance().sleepUntilTermination();
            std::exit(0);
        }
        else 
        {
            // parent process should track all child PIDs.
            pids.push_back(pid);
        }
    }   
    return 0;    
}

/**
 * @brief   The created child processes will eventuall terminate, based
 *          on a random condition. To show that a ad-hoch "re-connection"
 *          to the shared memory space is possible, this function is utilized
 *          to check that the number of user-requested processes is fulfilled.
 * 
 * @param   nr_process The number of user-requested processes.
 * @param   pids A reference to the vector structure, that holds all child PIDs.
 * @return  none
 */
void spawnProcess(size_t nr_process, std::vector<pid_t>& pids)
{
    while(true)
    {
        for (size_t i = 0; i < pids.size();)
        {
            int status = 0; 
            pid_t ret_code = waitpid(pids.at(i), &status, WNOHANG); 
            
            if (ret_code == 0)
            {
                // child is still running. continue to next PID.
                ++i;
            }
            else 
            {
                // remove child for vector table, as it does not live anymore.
                pids.erase(pids.begin()+i);
            }
        }

        size_t spawn_nr = nr_process - pids.size();
        if (spawn_nr > 0)
        {
            if (createProcess(spawn_nr, pids))
            {
                break;
            }
        }
    }
}

/**
 * @brief   Generic main function, which is the entry point of the exercise 
 *          application. 
 *  
 *  @return  The error code, where 0 means success, and other than that meaning failure.
 */
int main(void) 
{
    std::vector<pid_t> pids{};
    size_t nr_process = 0; 

    std::cout << "\x1B[2J\x1B[H" << std::endl; // clear the console
    printMessage(MsgTypes::INFO, "~~~~~~~~~~ IPC Exercise Task by Fikret Eren ~~~~~~~~~~");

    nr_process = getNrProcessByUsr(); 

    spawnProcess(nr_process, pids);
    
    waitForChildsTermination(pids);
    
    printMessage(MsgTypes::INFO, "~~~~~~~~~~ Exiting Application ~~~~~~~~~~");
    return 0;
}


#ifndef PROCESS_H
#define PROCESS_H

#include "util.hpp"
#include <random>
#include <limits>
#include <thread>

class Process 
{
private:
    IPCDatagramm* _ipc_msg; // will point to the package on the stack of the "main"-thread.
    std::mutex _ipc_msg_lock; 
    
    IPCDatagramm* _ipc_buffer; // will point to the memory mapped address of the package (inter-process).
    sem_t* _ipc_buffer_lock;

    std::thread _eva_thread;
    std::thread _work_thread;
    static std::atomic<bool> _force_kill;

    Process(); // this class is basically a singleton-pattern.    

    /**
     * @brief   This function will map the (shared) physical page to its virtual 
     *          address space, ensuring that read and write operations can be
     *          made. 
     * 
     * @return  Error code, where 0 means success, and other than that meaning failure.
     */
    int mapToPhyPage(void);

    /**
     * @brief   This helper function will create a random unsigned value
     *          which is between the user-defined <min> and <max> parameters.
     * 
     * @param   min The minimum value, which should be returned. 
     * @param   max The maximum value, which should be returned.
     * 
     * @return  The randomly generated number.
     */
    size_t getRandValue(size_t min, size_t max);

    /**
     * @brief   Showcasing that a "re-connection" to the shared memory
     *          data object is working, this functions job is to periodically
     *          check if the process should terminate based on a random
     *          condition. This function (thread) will wake the "main"-thread
     *          up, introducing a clean-up routine.
     * 
     * @return  none
     */
    void evaluateTermination(void);

    /**
     * @brief   This function will try to acquire the inter-process lock,
     *          protecting the shared memory space, and after reading and 
     *          printing it to the console, the data will be updated by
     *          the accessing process.
     * 
     * @return  none
     */
    void interCommunicator(void);
public:

    /**
     * @brief   Each process will obtain a package that is placed
     *          into the shared memory space. This function points
     *          the member variable of the process class to the 
     *          data package which will be sent. Additionally, a random
     *          string will be placed in that _data variable of the struct.
     * 
     * @param   packet  valid pointer reference to the package structure.
     * 
     * @return  none
     * 
     */
    void setMsgForIPC(IPCDatagramm* const packet);

    /**
     * @brief   Essentially putting the "main"-thread to sleep, till 
     *          the termination condition is met.
     * 
     * @return  none
     */
    void sleepUntilTermination(void);

    /**
     * @brief   Returns the singleton instance (thread-safe)
     * 
     * @return  Reference to the singleton process.  
     */
    static Process& getInstance(void); 

    // needed boilerplate for singleton pattern,
    // ensuring non-copyable object.
    Process(const Process& obj) = delete;
    void operator=(const Process&) = delete;
    
    ~Process(); 
};

#endif /* PROCESS_H */
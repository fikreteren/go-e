#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <mutex>
#include <fmt/format.h> 
#include <cstring>
#include <chrono>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

// The inter-process identifier, for "connecting" to the 
// shared memory space.
const char* const SHM_BlockIdentifier = "IPCSpace";
const char* const SHM_MemProt         = "IPCSpace_Lock";
const unsigned int SHM_BlockSize      = sysconf(_SC_PAGE_SIZE);


struct IPCDatagramm {
    // _pckg_id and _not_dirty (bit) are utilized as meta data for the
    // communication protocol. 
    unsigned int _pckg_id;
    bool _not_dirty;
    // _data is the actual payload, with 1kB size.
    char _data[1024ULL];
};

const std::vector<std::string> example_data{
    "I like coding",
    "The sun is bright",
    "The coffee smelled good",
    "I dont know what to write here",
    "I hope it doesnt rain tomorrow",
    "My wife wants to have a cat",
    "1+1=2",
    "The room temperature is: 42°C"
};

// Some helper functions and structures, for printing to stdout.
enum class MsgTypes {
    INFO = 0, 
    DEBUG,
    ERROR,
    DEATH,
    IPC,
    UNDEF
};

/**
 * @brief   This function be called instead of the std cout call,
 *          to have a clear console output.
 * 
 * @param   type The type of message, e.g. a error message. 
 * @param   msg A reference to the string, which should be printed.
 * @return  none
 */
inline void printMessage(MsgTypes type, const std::string& msg) 
{
    if (msg.empty())
    {
        return;
    }

    switch (type) 
    {
        case MsgTypes::INFO: 
            std::cout << "\033[34m[INFO]\033[0m      " << msg << std::endl;
            break;
        case MsgTypes::DEBUG:
            std::cout << "\033[35m[DEBUG]\033[0m     " << msg << std::endl;
            break;      
        case MsgTypes::ERROR:
            std::cout << "\033[31m[ERROR]\033[0m     " << msg << std::endl;
            break; 
        case MsgTypes::DEATH:
            std::cout << "\033[31m[DEATH]\033[0m     " << msg << std::endl;
            break;  
        case MsgTypes::IPC:
            std::cout << "\033[32m[IPC]\033[0m       " << msg << std::endl;
            break;          
        default:
            std::cout << "\033[36m[UNDEF]\033[0m     " << msg << std::endl;
            break;            
    }
} 

#endif /* UTIL_H */
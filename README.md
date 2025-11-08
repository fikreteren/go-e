
# Fikret Eren Coding Example

Dear **go-e** Team, 

first of fall thank you for the interesting assignment for the embedded engineer position. I've enjoyed working on it, especially as there is a lot of interpretation room, which is I assume intended by your side.\

I decided to take a more OS-related approach by utilizing the POSIX syscalls `fork()`, `shm_..()` and `mmap()` to create an inter-process memory object, which will
be read and modified by a user-definable number of processes. Another approach would be to use sockets or pipes. I have assumed that the "system" has a (POSIX) OS, as otherwise calling the above mentioned syscalls would lead to a error. However, if the embedded system
would running a "bare-metal" software, specifing a memory space via the linker script, which will be written to in the application could also be possible, since a MPSoC has 
more than 1 core, therefore a shared memory space is already imlemented in hardware. I am saying that because the terminology "multi-processing" does not
eventually mean that there are several hardware cores, but a unicore processor, which spawns multiple processors (=own address space), is also a "multi-processing" system.\

But yeah.. I hope we can discuss mroe about that in-person at the next opportunity.

Best Regards,\
*Fikret*


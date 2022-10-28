# MULTITHREADED OPERATING SYSTEM KERNEL

## Introduction
Project represents small, but functional multithreaded operating system  kernel. Threads are communicating with kernel via interrupts. For every kind of interrupt (external, by timer, by console) there is one common interrupt routine. Threads are working in client mode and every system call is considered as interrupt.

Operations that use system calls are:
 1) memory allocation/dealocation on heap (with operator new and delete)
 2) operations with semaphore (sem_open, sem_close, sem_wait, sem_signal)
 3) operations with threads (thread_exit, thread_dispatch, time_sleep)
 4) I/O operations (getc, putc)
 
 System calls are implemented like an interrupts with specific code.
 There are also other kinds of interrupts such as one from timer, from console etc.


## Intsructions
 These operations are implemented in C.
 Wrapper for C is written in C++ and represents the API.
 

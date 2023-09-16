//
// Created by os on 6/23/22.
//

#ifndef _syscall_cpp
#define _syscall_cpp
#include "syscall_c.h"
#include "../lib/hw.h"



void* operator new (size_t);
void operator delete (void*);
void *operator new[](size_t n);
void operator delete[](void *p) noexcept;


class Thread {
public:
    Thread (void (*body)(void*), void* arg);
    virtual ~Thread ();
    int start ();
    static void dispatch ();
    static int sleep (time_t);

protected:
    Thread ();
    virtual void run () {}
private:
    friend class TCB;
    thread_t myHandle;
    uint64* stack;
};


class Semaphore {
public:
    Semaphore (unsigned init = 1);
    virtual ~Semaphore ();
    int wait ();
    int signal ();
private:
    sem_t myHandle;
};

class PeriodicThread : public Thread {
protected:
    friend class TCB;
    PeriodicThread (time_t period);
    virtual void periodicActivation () {}
};

class Console {
public:
    static char getc ();
    static void putc (char);
};


#endif


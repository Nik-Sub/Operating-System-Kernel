#ifndef __syscall_c_h__
#define __syscall_c_h__

#include "../lib/hw.h"
//#include "../lib/console.h"
#include "tcb.h"
#include "MemoryAllocator.h"
#include "auxiliaryFunctions.hpp"

//extern "C" void* mem_allocS(size_t sizeInBlocks);
void* mem_alloc (size_t size);

//extern "C" void* mem_freeS();
int mem_free (void* mem);


int thread_create (
        thread_t* handle,
        void(*start_routine)(void*),
        void* arg,
        uint64* stack
);

void thread_dispatch ();

int thread_exit ();

int sem_open (
        sem_t* handle,
        unsigned init
);

int sem_close (sem_t handle);

int sem_wait (sem_t id);

int sem_signal (sem_t id);

int time_sleep (time_t);

void putc (char ch);

char getc ();

void* slabMemAlloc(char const* name, size_t sizeOfData);

void slabMemFree(char const* name, void* addr);


#endif
//
// Created by os on 6/23/22.
//

#include "../h/syscall_cpp.h"
#include "../h/printing.hpp"
#include "../h/SystemConsole.h"
void* operator new (size_t size){
    //__asm__ volatile ("addi sp, sp, -8");
    //__asm__ volatile ("sd a5, 0x00(sp)");
    void* ret = mem_alloc(size);
    //__asm__ volatile ("ld a5, 0x00(sp)");
    //__asm__ volatile ("addi sp, sp, 8");
    //if ((uint64)ret < 0x0000000080000000)
    //    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
    return ret;
}

void *operator new[](size_t n)
{
    void* ret = mem_alloc(n);
    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
    return ret;
}

void operator delete (void* addr){
    mem_free(addr);
}

void operator delete[](void *p) noexcept
{
    mem_free(p);
}

/*
void bla(void* a){}
int main() {
    __asm__ volatile("csrw stvec, %[adrFun]": : [adrFun]"r" (&interrHand));

    sem_t s;
    sem_open(&s, 1);
    s->block();

    return 0;
}
 */



Thread::Thread(void (*body)(void *), void *arg) {
    myHandle = nullptr;
    //thread_t * ptrToHandle = &myHandle;
    stack = (uint64*)slabMemAlloc("CacheThreadStacks", DEFAULT_STACK_SIZE);
    if (stack != nullptr) {
        thread_create(&myHandle, body, arg, stack);
    }
}

Thread::~Thread() {
    if (myHandle != nullptr)
        delete myHandle;
    slabMemFree("CacheThreadStacks", stack);
}

void Thread::dispatch() {
    thread_dispatch();
}


int Thread::start() {
    if (stack != nullptr)
        thread_create(&myHandle, TCB::wrapperForRun,this, stack);
    if (myHandle == nullptr) {
        //printString("Postalo je\n");
        //__asm__ volatile ("mv a0, %[var]": : [var]"r" (-1));
        return -1;
    }
    else {
        //__asm__ volatile ("mv a0, %[var]": : [var]"r" (0));
        return 0;
    }
}


Thread::Thread() {
    //thread_create(&myHandle, nullptr,nullptr);
    myHandle = nullptr;
    stack = (uint64*)slabMemAlloc("CacheThreadStacks", DEFAULT_STACK_SIZE);
}

int Thread::sleep(time_t time) {
    return time_sleep(time);
}


Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    delete myHandle;
}

int Semaphore::wait() {
    int ret = sem_wait(myHandle);
    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
    return ret;
}

int Semaphore::signal() {
    int ret = sem_signal(myHandle);
    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
    return ret;
}

PeriodicThread::PeriodicThread (time_t period){
    // we will not need handle for PeriodicThread
    thread_t tempHandle = nullptr;
    uint64 * stack = (uint64*)slabMemAlloc("CacheThreadStacks", DEFAULT_STACK_SIZE);
    thread_create(&tempHandle, TCB::wrapperForPeriodicActivation, this, stack);
    tempHandle->timeForPeriodicActivation = period;
    tempHandle->flagForPeriod = true;
}


void Console::putc (char c) {
    ::putc(c);
}

char Console::getc() {
    return ::getc();
}



/*
void f(void* p){
    while(true){}
}


int main(){
    __asm__ volatile("csrw stvec, %[adrFun]": : [adrFun]"r" (&interrHand));
    SystemConsole* s = new SystemConsole();
    Thread outputThread(s->funForOutputThread, nullptr);
    // changing mode
    __asm__ volatile("li a0, 0x03");
    __asm__ volatile("ecall");

    // if we dont have any thread in scheduler, we will always have this one
    Thread busyWait(f, nullptr);

    // because in thread dispatch i will put something in
    // TCB::running so it won't be nullptr ( it made error when we first time enter block() )
    thread_dispatch();
    putc('a');
    putc('a');
    putc('a');
    int c = getc();
    putc(c);
    putc('a');
    putc('a');

    return 0;
}
*/
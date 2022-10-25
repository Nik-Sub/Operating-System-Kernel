//
// Created by os on 6/23/22.
//

#include "../h/syscall_cpp.h"
//#include "../test/printing.hpp"
#include "../h/SystemConsole.h"
void* operator new (size_t size){
    return mem_alloc(size);
}
void operator delete (void* addr){
    mem_free(addr);
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
    //thread_t * ptrToHandle = &myHandle;
    thread_create(&myHandle, body, arg);
}

Thread::~Thread() {
    delete myHandle;
}

void Thread::dispatch() {
    thread_dispatch();
}


int Thread::start() {
    thread_create(&myHandle, TCB::wrapperForRun,this);
    return 0;
}


Thread::Thread() {
    //thread_create(&myHandle, nullptr,nullptr);
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
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

PeriodicThread::PeriodicThread (time_t period){
    // we will not need handle for PeriodicThread
    thread_t tempHandle = nullptr;
    thread_create(&tempHandle, TCB::wrapperForPeriodicActivation, this);
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
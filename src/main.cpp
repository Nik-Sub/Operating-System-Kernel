//
// Created by os on 8/19/22.
//
#include "../h/syscall_cpp.h"
#include "../h/SystemConsole.h"
extern int userMain();

void wrapperForUserMain(void* p){
    Semaphore* waitUsMainSem = (Semaphore*)p;
    userMain();
    waitUsMainSem->signal();
}




void main(){
    __asm__ volatile("csrw stvec, %[adrFun]": : [adrFun]"r" (&interrHand));
    SystemConsole* volatile s = new SystemConsole();
    Thread outputThread(s->funForOutputThread, nullptr);


    Semaphore waitUserMainSem(0);

    // changing mode
    __asm__ volatile("li a0, 0x03");
    __asm__ volatile("ecall");

    // TCB::running so it won't be nullptr ( it made error when we first time enter block() )
    thread_dispatch();

    // if we dont have any thread in scheduler, we will always have this one
    Thread busyWait(SystemConsole::busyWait, nullptr);

    // because in thread dispatch i will put something in

    Thread user(wrapperForUserMain,&waitUserMainSem);

    waitUserMainSem.wait();

}
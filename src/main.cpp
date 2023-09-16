//
// Created by os on 8/19/22.
//
#include "../h/syscall_cpp.h"
#include "../h/SystemConsole.h"
#include "../h/SlabAllocator.h"
extern int userMain();

void wrapperForUserMain(void* p){
    Semaphore* waitUsMainSem = (Semaphore*)p;
    userMain();
    waitUsMainSem->signal();
}

void wrapper(void* p){
    while(true){
        thread_dispatch();
    }
}

/*
class BusyWait: public Thread{
public:
    BusyWait(): Thread(){}
    virtual void run() override{
        while(true){
            thread_dispatch();
        }
    }
};
 */



void main(){
    //BuddyAllocator* bA = BuddyAllocator::getInstance();
    //void* ptr = bA->buddyMemAlloc(16);
    //bA->buddyMemFree(ptr, 16);
    /*
    Cache* c = new Cache("prviCache", 16);
    Cache* c2 = new Cache("drugiCache", 16);
    delete c2;
    //c2 = nullptr;
    c2 = new Cache("TreciCache", 16);
    delete c;
    delete c2;
    c2 = new Cache("drugiCache", 16);
    */
    //SlabAllocator* sA = new SlabAllocator();
    //delete sA;

    __asm__ volatile("csrw stvec, %[adrFun]": : [adrFun]"r" (&interrHand));
    SystemConsole* volatile s = new SystemConsole();


    Thread outputThread(s->funForOutputThread, nullptr);


    Semaphore waitUserMainSem(0);


    // changing mode
    __asm__ volatile("li a0, 0x03");
    //__asm__ volatile("ecall");

    // TCB::running so it won't be nullptr ( it made error when we first time enter block() )
    thread_dispatch();

    //**
    // if we dont have any thread in scheduler, we will always have this one
    Thread busyWait(s->busyWait, nullptr);
    //BusyWait* busyWait = new BusyWait();
    //busyWait->start();
    //**

    // because in thread dispatch i will put something in


    //**
    Thread user(wrapperForUserMain,&waitUserMainSem);


    waitUserMainSem.wait();
    //**

}
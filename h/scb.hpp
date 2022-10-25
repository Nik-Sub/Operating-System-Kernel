//
// Created by os on 8/12/22.
//

#ifndef PROJECT_BASE_V1_1_SCB_HPP
#define PROJECT_BASE_V1_1_SCB_HPP

#include "List.hpp"
#include "tcb.h"
#include "MemoryAllocator.h"

class SCB{
private:
    // value of semaphore
    int value;
    List<TCB> blockedThreads;
    //save context before put thread in blocked queue
    TCB::Context* saveContext(TCB::Context*);
    // to change context from blocked to another thread
    void changeContext();

    SCB(unsigned val): value(val){}
    void putThreadInBlocked();
    TCB* getThreadFromBlocked();
    int getValForJump();
public:
    friend class Semaphore;
    friend class Riscv;
    friend class SystemConsole;
    void block();
    void deblock();
    static SCB* createSemaphore(unsigned val);
    static int semClose(SCB* sem);
    int getValue(){
        return value;
    }
};



#endif //PROJECT_BASE_V1_1_SCB_HPP

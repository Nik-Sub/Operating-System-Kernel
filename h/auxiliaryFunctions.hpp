//
// Created by os on 8/9/22.
//

#ifndef PROJECT_BASE_V1_1_AUXILIARYFUNCTIONS_HPP
#define PROJECT_BASE_V1_1_AUXILIARYFUNCTIONS_HPP

#include "../h/MemoryAllocator.h"
//#include "../lib/console.h"
#include "tcb.h"
#include "scb.hpp"

typedef TCB* thread_t;
typedef SCB* sem_t;
typedef unsigned long time_t;
const int EOF = -1;

extern "C" void interrHand();

class Riscv {
public:
    static void interruptRoutine();
    //extern "C" static void chResume();
    //extern "C" void chhResume();

    static void popSppSpie();

    //static void interruptHandler();

    static void *forEveryFunc(int codeOfFunc, void *par);

    //static void lock(unsigned* lck);
    //static void unlock(unsigned* lck);

};


#endif //PROJECT_BASE_V1_1_AUXILIARYFUNCTIONS_HPP

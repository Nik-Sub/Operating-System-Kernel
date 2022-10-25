//
// Created by os on 8/8/22.
//

#ifndef PROJECT_BASE_V1_1_SCHEDULER_HPP
#define PROJECT_BASE_V1_1_SCHEDULER_HPP
#include "List.hpp"
//#include "../lib/console.h"

class TCB;

struct SleepThreads{
    unsigned long time;
    TCB* thread;
    SleepThreads* next;
    SleepThreads(unsigned long timee, TCB* threadd, SleepThreads* nextt = nullptr):time(timee), thread(threadd), next(nextt){}
};

class Scheduler
{
private:
    static List<TCB> readyThreadQueue;
    static SleepThreads* sleepThreadsHead;
public:
    friend class Riscv;
    friend class SCB;
    friend class SystemConsole;
    // methods for readyThreadQueue
    static TCB *get();
    static void put(TCB *tcb);
    // methods for sleepThreadQueue
    static SleepThreads *getSleep();
    static void putSleep(unsigned long time);

};



#endif //PROJECT_BASE_V1_1_SCHEDULER_HPP

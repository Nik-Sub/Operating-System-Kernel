//
// Created by os on 8/8/22.
//

#include "../h/Scheduler.hpp"
#include "../h/tcb.h"

List<TCB> Scheduler::readyThreadQueue;
SleepThreads* Scheduler::sleepThreadsHead = nullptr;

TCB *Scheduler::get() {
    return Scheduler::readyThreadQueue.removeFirst();
}

void Scheduler::put(TCB *Tcb) {
    Scheduler::readyThreadQueue.addLast(Tcb);
    //__putc('O');
}

SleepThreads *Scheduler::getSleep() {
    SleepThreads* temp = sleepThreadsHead;
    if (sleepThreadsHead != nullptr)
        sleepThreadsHead = sleepThreadsHead->next;
    return temp;
}

void Scheduler::putSleep(unsigned long time) {
    SleepThreads* newSleepThread = new SleepThreads(time, TCB::running);
    // its empty
    if (sleepThreadsHead == nullptr){
        sleepThreadsHead = newSleepThread;
    }
    else{
        SleepThreads* cur = sleepThreadsHead;
        SleepThreads* prev = nullptr;
        while (cur){
            if (time < cur->time){
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        // if we have curr, then insert elem on the left side of cur
        if (cur){
            newSleepThread->next = cur;
            cur->time -= time;
            if (prev != nullptr)
                prev->next = newSleepThread;
            else{
                sleepThreadsHead = newSleepThread;
            }
        }
        // new elem will be insert on the right side with relative time to prev
        else{
            prev->next = newSleepThread;
            newSleepThread->time = time - prev->time;
            newSleepThread->next = nullptr;
        }
    }
}

//
// Created by os on 8/9/22.
//

#ifndef PROJECT_BASE_V1_1_TCB_H
#define PROJECT_BASE_V1_1_TCB_H

#include "../lib/hw.h"
#include "Scheduler.hpp"

// Thread Control Block
class TCB
{
public:
    friend class Riscv;
    friend class Thread;
    friend class PeriodicThread;
    friend class SCB;

    ~TCB() {
        //__putc('O');
        delete stack;
    }

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    bool isSleeping() const { return sleeping; }

    void setSleeping(bool value) { sleeping = value; }

    uint64 getTimeSlice() const { return timeSlice; }
    void setTimeSlice(time_t period) { timeSlice = period; }

    using Body = void (*)(void*);

    static TCB *createThread(Body body, void* arg, uint64* stack);

    static void yield();

    static TCB *running;

    static void dispatch();

    int getRetValForWait() const{
        return context.retValForWait;
    }

    // need this for if in block() in scb
    int retValForJump(){
        return context.valForJmp;
    }


private:
    // need wrapper because of type Body, so we need wraper for run()  !!
    static void wrapperForRun (void* t);
    // need wrapper beacuse of type Body, so we need wrapper for periodicActivation() !!
    static void wrapperForPeriodicActivation(void* t);
    TCB(Body body, void* arg, uint64 *stackk) :
            body(body),
            stack(stackk),
            context({(uint64) &threadWrapper,
                     (uint64)stackk, (int)0, (int)0
                    }),
            finished(false)
    {
        this->arg = arg;
        // because we will make one with nullptr body on the start, so we dont put that thread twice
        if (body != nullptr) {
            Scheduler::put(this);

        }
    }

    struct Context
    {
        uint64 ra;
        uint64 sp;
        //we will need this value to not enter if in block() when we return second time there
        int valForJmp;
        // will be 1 if we closed semaphore while there were blocked threads
        int retValForWait;
    };

    // body of thread
    Body body;

    void* arg;
    // need this flag because if thread didn not start,
    // we wont change mode (to user) in changeContext at
    // scb class
    bool started = false;
    // time for periodic activation for this thread,
    // (after it finishes, it will sleep for that time, and then activate again
    time_t timeForPeriodicActivation;
    bool flagForPeriod = false; // to know if this thread is periodic or not
    uint64 *stack;
    Context context;
    uint64 timeSlice = DEFAULT_TIME_SLICE;
    bool finished;
    bool sleeping = false;

    static void threadWrapper();

    static void contextSwitch(Context *oldContext, Context *runningContext);
    // need this when oldRunning is periodic thread,
    // so we already manage ra and sp of it so we can not
    // use context switch function
    static void contextSwitchWhenOldRunningIsPeriodic(Context* newContext);

    static uint64 timeSliceCounter;

    void setBody(Body body, void* arg);

    // need this field because of output thread before userMain
    // that thread must working in supervisor mode
    uint64 firstSstatus;
};




#endif //PROJECT_BASE_V1_1_TCB_H

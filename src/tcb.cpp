//
// Created by os on 8/9/22.
//
#include "../h/tcb.h"
#include "../h/auxiliaryFunctions.hpp"
//#include "../test/printing.hpp"
#include "../h/syscall_cpp.h"
#include "../h/SlabAllocator.h"


TCB *TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;
uint64 TCB::pomid = 1;

TCB *TCB::createThread(TCB::Body body, void *arg, uint64 *stack) {
    //printInt((uint64)(arg));
    if (stack == nullptr) {
        __asm__ volatile ("mv a0, %[var]": : [var]"r" (0));
        return 0;
    }
    else {
        //uint64 volatile oldSSTATUS;
        //__asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));

        TCB* ret = new TCB(body, arg, stack);
        __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
        //__asm__ volatile ("csrw sstatus, %[var]": :[var]"r" (oldSSTATUS));
        return ret;
    }
}

void TCB::dispatch() {
    //uint64 volatile oldSSTATUS;
    //__asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));
    TCB* oldRunning = running;
    // we need one PCB to save ra and sp from the first time we got here (this tcb will represent main)
    if (!oldRunning) {
        //thread_create(&oldRunning, nullptr, nullptr);
        oldRunning = new TCB(nullptr, nullptr, nullptr);
        // main mora da se izvrsava u korisnickom rezimu
        //uint64 mask = (0x100);
        //oldRunning->firstSstatus = oldSSTATUS;
    }
    //oldRunning->firstTimeSubstitute = false;

    if (!(oldRunning->isFinished() || oldRunning->isSleeping()))
        Scheduler::put(oldRunning);
    if (oldRunning->isFinished() && oldRunning->flagForPeriod){
        oldRunning->context.ra = (uint64) &threadWrapper;
        Scheduler::putSleep(oldRunning->timeForPeriodicActivation);
        TCB::running->setSleeping(true);
    }

    running = Scheduler::get();
    //int t = 3;
    //t += 1;

    /*
    if (!running->firstTimeSubstitute) {
        //__asm__ volatile ("csrw sstatus, %[var]": :[var]"r"(oldSSTATUS));
    }
    else{
        running->firstTimeSubstitute = false;
        __asm__ volatile ("csrw sstatus, %[var]": :[var]"r"(running->firstSstatus));
    }
     */

    if(!(oldRunning->isFinished() && oldRunning->flagForPeriod)) {
        contextSwitch(&oldRunning->context, &running->context);
    }
    else{
        // now change context to another
        // we must do change this way because we already saved context
        // for oldRunning
        __asm__ volatile ("mv a1, %[par]": : [par]"r" (&running->context));
        __asm__ volatile ("ld ra, 0x00(a1)");
        __asm__ volatile ("ld sp, 0x08(a1)");
    }
}

void TCB::contextSwitch(TCB::Context * oldContext, TCB::Context * runningContext) {
    // arguments passed through a0 and a1
    //a0-old, a1-new
    __asm__ volatile ("sd ra, 0x00(a0)");
    __asm__ volatile ("sd sp, 0x08(a0)");
    __asm__ volatile ("sd ra, 0x10(a0)");
    __asm__ volatile ("sd ra, 0x18(a0)");
    //__asm__ volatile ("sd sp, 0x18(a0)");


    //if (runningContext->ra >= 0x0000000080008000) { // runningContext->ra == 0x0000000080008be8 || runningContext->ra == 0x0000000080008b88 || runningContext->ra == 0x000000008000861c)
        __asm__ volatile ("ld ra, 0x18(a1)");
        //__asm__ volatile ("ld sp, 0x18(a1)");
    //}
    //else {
    //    __asm__ volatile ("ld ra, 0x00(a1)");
    //}
    __asm__ volatile ("ld sp, 0x08(a1)");


}

void TCB::threadWrapper() {

    //__asm__ volatile ("mv gp, %[var]": : [var]"r" (0));
    //__asm__ volatile ("mv tp, %[var]": : [var]"r" (0));
    // returning fields to the start value
    // beacuse of periodic threads
    running->started = false;
    running->finished = false;

    //__asm__ volatile ("mv gp, %[var]": : [var]"r" (0));
    //__asm__ volatile ("mv tp, %[var]": : [var]"r" (0));
    //we want thread to work in user mode
    Riscv::popSppSpie();
    __asm__ volatile ("mv gp, %[var]": : [var]"r" (0));
    __asm__ volatile ("mv tp, %[var]": : [var]"r" (0));
    // we need this flag because of change context
    // at scb class
    running->started = true;
    //printInt((uint64)(running->arg));
    running->body(running->arg);
    running->finished = true;
    yield();
}

void TCB::yield() {
    //let's say 0x00 is code for sync context switch
    __asm__ volatile ("li a0, 0x00");
    __asm__ volatile ("ecall");
}

void TCB::setBody(TCB::Body body, void *arg) {
    this->body = body;
    this->arg = arg;
}

void TCB::wrapperForRun(void *t)  {
    if(t)((Thread*)t)->run();
}

void TCB::wrapperForPeriodicActivation(void* t){
    if(t)((PeriodicThread*)t)->periodicActivation();
}

void TCB::contextSwitchWhenOldRunningIsPeriodic(TCB::Context *newContext) {
    __asm__ volatile ("ld ra, 0x00(a0)");
    __asm__ volatile ("ld sp, 0x08(a0)");
}

void *TCB::operator new(size_t s) {
    return SlabAllocator::getInstance()->alloc("CacheTCB", s);
}

void TCB::operator delete(void *p) {
    if (p == nullptr)
        return;
    slabMemFree("CacheTCB", p);
}

TCB::~TCB() {
    delete stack;
}

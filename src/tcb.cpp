//
// Created by os on 8/9/22.
//
#include "../h/tcb.h"
#include "../h/auxiliaryFunctions.hpp"
//#include "../test/printing.hpp"
#include "../h/syscall_cpp.h"
TCB *TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;

TCB *TCB::createThread(TCB::Body body, void *arg, uint64 *stack) {
    //printInt((uint64)(arg));
    return new TCB(body, arg, stack);
}

void TCB::dispatch() {
    uint64 volatile oldSSTATUS;
    __asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));
    TCB* oldRunning = running;
    // we need one PCB to save ra and sp from the first time we got here
    if (!oldRunning) {
        oldRunning = new TCB(nullptr, nullptr, nullptr);
    }

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
    __asm__ volatile ("csrw sstatus, %[var]": :[var]"r" (oldSSTATUS));
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

void TCB::contextSwitch(TCB::Context *oldContext, TCB::Context *runningContext) {
    // arguments passed through a0 and a1
    //a0-old, a1-new
    __asm__ volatile ("sd ra, 0x00(a0)");
    __asm__ volatile ("sd sp, 0x08(a0)");


    __asm__ volatile ("ld ra, 0x00(a1)");
    __asm__ volatile ("ld sp, 0x08(a1)");
}

void TCB::threadWrapper() {
    // returning fields to the start value
    // beacuse of periodic threads
    running->started = false;
    running->finished = false;
    //we want thread to work in user mode
    Riscv::popSppSpie();
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
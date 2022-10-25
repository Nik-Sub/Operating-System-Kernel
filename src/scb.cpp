//
// Created by os on 8/12/22.
#include "../h/scb.hpp"
TCB::Context* SCB::saveContext(TCB::Context* context) {
    //parametar is in a1, not in a0!!!!!!!!!
    //__asm__ volatile ("mv a0, %[par]": : [par]"r" (context));;
    // saving context of running thread
    __asm__ volatile ("sd ra, 0x00(a1)");
    __asm__ volatile ("sd sp, 0x08(a1)");
    // val is 0 because we want to enter if from blocked()
    // because we need to switch context
    unsigned val = 0;
    __asm__ volatile ("mv t1, %[par]": : [par]"r" (val));
    __asm__ volatile ("sd t1, 0x10(a1)");
    return context;

}

void SCB::block() {
    saveContext(&(TCB::running->context));
    int val = getValForJump();
    if ( val == 0){
        putThreadInBlocked();
        TCB::running = Scheduler::get();
        changeContext();
    }
}

void SCB::deblock() {
    TCB* unblockedThread = getThreadFromBlocked();
    // we must change value because we do not want to enter if in block()!!
    unblockedThread->context.valForJmp = 1;
    Scheduler::put(unblockedThread);
}

SCB *SCB::createSemaphore(unsigned val) {
    return new SCB(val);
}

int SCB::semClose(SCB *sem) {
    TCB* thread = nullptr;
    while ((thread = sem->blockedThreads.removeFirst())){
        thread->context.retValForWait = -1;
        Scheduler::readyThreadQueue.addLast(thread);
    }
    return MemoryAllocator::getInstance()->freeMem(sem);
}

void SCB::putThreadInBlocked() {
    blockedThreads.addLast(TCB::running);
}

TCB* SCB::getThreadFromBlocked(){
    return blockedThreads.removeFirst();
}

int SCB::getValForJump(){
    return TCB::running->retValForJump();
}

void SCB::changeContext() {
    // now we need to change context to the running thread
    /*if (TCB::running->started) {
        __asm__ volatile ("mv ra, %[par]": : [par] "r" (TCB::running->context.ra));
        __asm__ volatile ("mv sp, %[par]": : [par] "r" (TCB::running->context.sp));
        // mask for switching resume to user
        uint64 mask = 0x100;
        __asm__ volatile("csrs sstatus, %[par]": : [par]"r"(mask));
        // mask for changing value of spie ( turning on external interrupts in user mode)
        mask = 0x01 << 5;
        __asm__ volatile("csrs sstatus, %[par]": : [par]"r"(mask));
    }
    else{
     */
        // function popSppSpie will do the rest
        __asm__ volatile ("mv ra, %[par]": : [par] "r" (TCB::running->context.ra));
        __asm__ volatile ("mv sp, %[par]": : [par] "r" (TCB::running->context.sp));

    //}
}

//

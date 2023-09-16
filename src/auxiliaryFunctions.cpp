//
// Created by os on 8/9/22.
//
/*
void print2(size_t n) {
    if (n < 0) {
        __putc('-');
        n = -n;
    }

// Remove the last digit and recur
    if (n / 10)
        print2(n / 10);

// Print the last digit
    __putc(n % 10 + '0');
}
 */
#include "../h/auxiliaryFunctions.hpp"
#include "../h/syscall_c.h"
#include "../h/SystemConsole.h"


void Riscv::popSppSpie()
{

    __asm__ volatile("csrw sepc, ra");
    // mask for switching resume to user
    uint64 mask = 0x100;
    // output thread must working in supervisor mode
    if (!(TCB::running->firstSstatus & 0x100))
        __asm__ volatile("csrc sstatus, %[par]": : [par]"r" (mask));
    else
        __asm__ volatile("csrs sstatus, %[par]": : [par]"r" (mask));

    // mask for changing value of spie ( turning on external interrupts in user mode)
    mask = 0x01 << 5;
    __asm__ volatile("csrs sstatus, %[par]": : [par]"r" (mask));

    __asm__ volatile("sret");
}

//int globW = 0;
//int globS = 0;

void Riscv::interruptRoutine(){
    // sstatusVar and codeOfFunc and result of first if will be saved in registars a3, a4, a5
    // i need a3 and a4 to be saved on stack (f.e. for thread_create)
    __asm__ volatile ("addi sp, sp, -40");
    __asm__ volatile ("sd a3, 0x00(sp)");
    __asm__ volatile ("sd a4, 0x08(sp)");
    __asm__ volatile ("sd a5, 0x10(sp)");
    __asm__ volatile ("sd a0, 0x18(sp)");
    __asm__ volatile ("sd a1, 0x20(sp)");


    uint64 volatile a00;
    __asm__ volatile ("mv %[var], a0" : [var] "=r" (a00));

    uint64 volatile oldSEPC;
    uint64 volatile oldSEPC2;
    uint64 volatile oldSSTATUS;

    uint64 sstatusVar;
    __asm__ volatile ("csrr %[var], scause": [var]"=r" (sstatusVar));
    if (sstatusVar == 2)
        __asm__ volatile ("csrr %[var], scause": [var]"=r" (sstatusVar));
    if (sstatusVar == 5)
        __asm__ volatile ("csrr %[var], scause": [var]"=r" (sstatusVar));
    if (sstatusVar == 7)
        __asm__ volatile ("csrr %[var], scause": [var]"=r" (sstatusVar));
    if (sstatusVar == (0x08UL) || sstatusVar == (0x09UL) ) {
        // restoring values before check with if above
        __asm__ volatile ("ld a3, 0x00(sp)");
        __asm__ volatile ("ld a4, 0x08(sp)");
        __asm__ volatile ("ld a5, 0x10(sp)");
        // dohvatanje koda funkcije
        uint64 volatile codeOfFunc;
        __asm__ volatile ("mv %[code], a0" : [code] "=r" (codeOfFunc));
        // in some system calls we will have system calls too,
        // so we need to save current sepc and sstatus
        //uint64 volatile oldSEPC;
        //uint64 volatile oldSSTATUS;
        bool changeToUserMode = false;
        __asm__ volatile ("csrr %[var], sepc": [var]"=r"(oldSEPC));
        __asm__ volatile ("csrr %[var], sepc": [var]"=r"(oldSEPC2));
        __asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));
        switch (codeOfFunc) {
            // yield
            case 0x00: {
                // ecall is processed
                __asm__ volatile ("csrc sip, 0x02");
                // synchro context switch
                TCB *oldRunning = TCB::running;

                // we need one TCB to save ra and sp from the first time we got here
                if (!oldRunning) {
                    oldRunning = new TCB(nullptr, nullptr, nullptr);
                }

                if (!(oldRunning->isFinished() || oldRunning->isSleeping()))
                    Scheduler::put(oldRunning);
                if (oldRunning->isFinished() && oldRunning->flagForPeriod){
                    oldRunning->context.ra = (uint64) &TCB::threadWrapper;
                    Scheduler::putSleep(oldRunning->timeForPeriodicActivation);
                    TCB::running->setSleeping(true);
                }


                TCB::running = Scheduler::get();

                if(!(oldRunning->isFinished() && oldRunning->flagForPeriod)) {
                    TCB::contextSwitch(&oldRunning->context, &TCB::running->context);
                }
                else{
                    // now change context to another
                    // we must do change this way because we already saved context
                    // for oldRunning
                    TCB::contextSwitchWhenOldRunningIsPeriodic(&TCB::running->context);
                }
                break;
            }
            // mem_alloc
            case 0x01:{
                //__putc('M');
                void* sizeV;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (sizeV));
                size_t* size = (size_t*)sizeV;
                //print2(*size);
                void* addr = MemoryAllocator::getInstance()->memAlloc(*size);
                __asm__ volatile ("mv a0, %[var]": : [var]"r" (addr));
                break;

            }
            // mem_free
            case 0x02:{
                void* addr;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (addr));
                int retVal = MemoryAllocator::getInstance()->freeMem(addr);
                void* ret = nullptr;
                // succesfull
                if(retVal == 0){
                    // i will use any ptr different from nullptr
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (addr));
                }
                    //unsuccesfull
                else{
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (ret));
                }
                break;

            }
            // just change mode
            case 0x03:{
                changeToUserMode = true;
                // mask for switching resume to user
                uint64 mask = 0x100;
                __asm__ volatile("csrc sstatus, %[par]": : [par]"r" (mask));
                // bit SPIE
                __asm__ volatile("li t1, 0x20");
                __asm__ volatile("csrs sstatus, t1");

                break;
            }
            // thread_create
            case 0x11:{
                thread_t* handle;
                TCB::Body body;
                void* arg;
                uint64* stack_space; // last place of stack
                __asm__ volatile ("mv %[var], a1": [var]"=r" (handle));
                __asm__ volatile ("mv %[var], a2": [var]"=r" (body));
                __asm__ volatile ("ld %[var], 0x00(sp)": [var]"=r" (arg));
                __asm__ volatile ("ld %[var], 0x08(sp)": [var]"=r" (stack_space));

                *handle = TCB::createThread(body, arg, stack_space);
                if (*handle != nullptr)
                    (*handle)->firstSstatus = oldSSTATUS;
                break;
            }
            // thread_exit
            case 0x12:{
                // we will have system calls, so we need to save some csr registers
                TCB::running->setFinished(true);

                int retVal = mem_free(TCB::running->stack);

                // succesfull
                if(retVal != 0){
                    // we can do dispatch
                    TCB::dispatch();
                }
                //unsuccesfull
                else{
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (nullptr));
                }

                break;
            }
            // thread_dispatch
            case 0x13: {

                TCB::dispatch();
                __asm__ volatile ("ld a0, 0x18(sp)");
                __asm__ volatile ("ld a1, 0x20(sp)");
                break;
            }
            // sem_open
            case 0x21:{
                sem_t* handle;
                unsigned init;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (handle));
                __asm__ volatile ("mv %[var], a2": [var]"=r" (init));

                *handle = SCB::createSemaphore(init);


                if (*handle != nullptr){
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (handle));
                }
                else{
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (nullptr));
                }
                break;
            }
            // sem_close
            case 0x22:{
                sem_t handle;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (handle));

                int retVal = SCB::semClose(handle);

                if (retVal == 0) {
                    // passing any ptr
                    __asm__ volatile ("mv a0, %[var]": : [var]"r"(&retVal));
                }
                else{
                    __asm__ volatile ("mv a0, %[var]": : [var]"r"(nullptr));
                }
                break;
            }
            // sem_wait
            case 0x23:{
                sem_t handle;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (handle));

                if (handle == nullptr){
                    //globW++;
                }

                handle->value--;
                if (handle->value < 0) {
                    handle->block();
                    __asm__ volatile ("ld a0, 0x18(sp)");
                    __asm__ volatile ("ld a1, 0x20(sp)");
                    /*
                    // we were changed sstatus reg so just use oldSEPC
                    // return values of sepc and sstatus
                    __asm__ volatile ("csrw sepc, %[var]": :[var]"r" (oldSEPC));

                    // inc sepc because it points on ecall again
                    __asm__ volatile ("csrr x6, sepc");
                    __asm__ volatile ("addi x6, x6, 4");
                    __asm__ volatile ("csrw sepc, x6");

                    // clear bit for ecall
                    __asm__ volatile ("csrc sip, 0x02");
                    __asm__ volatile ("ret");
                     */
                }


                break;
            }
            // sem_signal
            case 0x24:{
                sem_t handle;
                __asm__ volatile ("mv %[var], a1": [var]"=r"(handle));
                if (handle == nullptr){
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (nullptr));
                    //globS++;
                }
                handle->value = handle->value + 1;
                if (handle->value <= 0) {
                    handle->deblock();
                }

                //successfull
                // passing any ptr
                __asm__ volatile ("mv a0, %[var]": : [var]"r" (&handle));
                break;
            }
            // time_sleep
            case 0x31:{
                time_t* time;
                __asm__ volatile ("mv %[var], a1": [var]"=r"(time));
                if (*time != 0) {
                    Scheduler::putSleep(*time);
                    TCB::running->setSleeping(true);
                }

                //TCB::dispatch();
                thread_dispatch();

                //successfull
                // passing any ptr
                __asm__ volatile ("mv a0, %[var]": : [var]"r" (TCB::running));
                break;
            }
            // putc
            case 0x41:{
                //__asm__ volatile("csrs sstatus, 0x02");
                //__asm__ volatile("csrc sstatus, 0x02");
                uint8* charForPut;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (charForPut));
                if (*charForPut == '\r')
                    *charForPut = '\n';
                SystemConsole::outputBuffer->put((int)(*charForPut));
                if (SystemConsole::outputSem->getValue() <= 0){
                    //sem_signal(SystemConsole::outputSem);
                }
                break;
            }
            // getc
            case 0x42:{
                char* retVal = (char*)mem_alloc(sizeof(char));
                if (retVal == nullptr){
                    __asm__ volatile ("mv a0, %[var]": : [var]"r" (&EOF));
                }
                *retVal = SystemConsole::inputBuffer->get();
                __asm__ volatile ("mv a0, %[var]": : [var]"r" (retVal));
                break;
            }
            // SlabAllocator - mem_alloc
            case 0x50:{
                //__putc('M');
                char const* nameOfCache;
                size_t sizeOfData;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (nameOfCache));
                __asm__ volatile ("mv %[var], a2": [var]"=r" (sizeOfData));

                void* addr = SlabAllocator::getInstance()->alloc(nameOfCache, sizeOfData);
                __asm__ volatile ("mv a0, %[var]": : [var]"r" (addr));
                break;

            }
            // SlabAllocator - mem_free
            case 0x51:{
                char const* nameOfCache;
                void* addr;
                __asm__ volatile ("mv %[var], a1": [var]"=r" (nameOfCache));
                __asm__ volatile ("mv %[var], a2": [var]"=r" (addr));
                SlabAllocator::getInstance()->free(nameOfCache, addr);

                break;

            }
            default:{
                break;
            }


        }
        // return values of sepc and sstatus
        if (oldSEPC != oldSEPC2)
            __asm__ volatile ("csrw sepc, %[var]": :[var]"r" (oldSEPC2));
        else
            __asm__ volatile ("csrw sepc, %[var]": :[var]"r" (oldSEPC));
        if (!changeToUserMode)
            __asm__ volatile ("csrw sstatus, %[var]": :[var]"r" (oldSSTATUS));

        // inc sepc because it points on ecall again
        __asm__ volatile ("csrr x6, sepc");
        __asm__ volatile ("addi x6, x6, 4");
        __asm__ volatile ("csrw sepc, x6");

        // clear bit for ecall
        __asm__ volatile ("csrc sip, 0x02");
        //__putc('Y');
    }
        // timer
    else if (sstatusVar == (0x01UL<<63 | 0x01)){
        // ako je tajmer, onda nisam usao sa ecall pa ne treba da se inkrementira sepc
        __asm__ volatile ("csrc sip, 0x02");
        //uint64 volatile oldSEPC;
        //uint64 volatile oldSSTATUS;
        __asm__ volatile ("csrr %[var], sepc": [var]"=r"(oldSEPC));
        __asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));

        SleepThreads* temp = Scheduler::getSleep();
        SleepThreads* futureHead = temp;
        // checking if we have some threads to awake
        if (temp){
            temp->time--;
            if (temp->time == 0){
                temp->thread->setSleeping(false);
                Scheduler::put(temp->thread);
                // checking if we have more threads with the same time
                temp = temp->next;
                futureHead = temp;
                while (temp){
                    if (temp->time == 0){
                        temp->thread->setSleeping(false);
                        Scheduler::put(temp->thread);
                        temp = temp->next;
                        futureHead = temp;
                    }
                    else
                        break;
                }
            }
        }
        Scheduler::sleepThreadsHead = futureHead;

        // maybe we will not have other thread (if we enter this before we make threads)
        if (Scheduler::readyThreadQueue.peekFirst()) {
            // async dispatch
            // maybe we will enter this when we dont have other threads
            if (TCB::running != nullptr) {
                TCB::timeSliceCounter++;
                if (TCB::timeSliceCounter == TCB::running->getTimeSlice()) {
                    TCB::timeSliceCounter = 0;
                    //TCB::dispatch();
                    thread_dispatch();
                    //__asm__ volatile ("ld a0, 0x00(sp)");
                }
            } else {
                //TCB::dispatch();
                thread_dispatch();
                //__asm__ volatile ("ld a0, 0x00(sp)");
            }
        }
        __asm__ volatile ("ld a0, 0x18(sp)");
        __asm__ volatile ("ld a1, 0x20(sp)");




        // return values of sepc and sstatus
        __asm__ volatile ("csrw sepc, %[var]": :[var]"r" (oldSEPC));
        __asm__ volatile ("csrw sstatus, %[var]": :[var]"r" (oldSSTATUS));
        //__putc('T');
    }
        // external interrupts
    else if (sstatusVar == (0x01UL<<63 | 0x09)){
        //uint64 volatile oldSEPC;
        //uint64 volatile oldSSTATUS;
        __asm__ volatile ("csrr %[var], sepc": [var]"=r"(oldSEPC));
        __asm__ volatile ("csrr %[var], sstatus": [var]"=r"(oldSSTATUS));


        //uint64 maskForExtInt = 1<<9;
        //__asm__ volatile ("csrc sip, %[var]": : [var]"r" (maskForExtInt));
        if (plic_claim() == 0x0a) {
            plic_complete(10);

            //if (*((uint8 *) CONSOLE_STATUS) & (1 << 5)) {
                //SystemConsole::getInstance()->funForOutputThread(nullptr);
                //if (SystemConsole::extIntSemForOutput->getValue() <= 0) {
                    //sem_signal(SystemConsole::extIntSemForOutput);
                //}
            //}
            while (*((uint8 *) CONSOLE_STATUS) & 0x01) {
                SystemConsole::inputBuffer->put(*((uint8 *) CONSOLE_RX_DATA));
            }


        }
        //returning a0 because it wont be returned with interrupt handler
        __asm__ volatile ("ld a0, 0x18(sp)");
        // return values of sepc and sstatus
        __asm__ volatile ("csrw sepc, %[var]": :[var]"r" (oldSEPC));
        __asm__ volatile ("csrw sstatus, %[var]": :[var]"r" (oldSSTATUS));

    }
    //error
    else{
        //printString("Error");
    }

    // delee a3 and a4
    __asm__ volatile ("addi sp, sp, 40");
    // let timer make interrupts
    //__asm__ volatile("li t1, 0x20");
    //__asm__ volatile("csrs sstatus, t1");
}

void* Riscv::forEveryFunc(int codeOfFunc, void* par){
    //print2(*((size_t*)par));
    //__putc('\n');
    /*
    // push arg on stack beacuse we need code of func in a0
    __asm__ volatile ("addi sp, sp, -8");
    __asm__ volatile ("sd a0, 0x00(sp)");

    __asm__ volatile ("mv a0, %[code]": :[code] "r" (codeOfFunc));
    __asm__ volatile ("ld a1, 0x00(sp)");
    __asm__ volatile ("addi sp, sp, 8");
    */
    __asm__ volatile ("ecall");
    //__putc('K');
    void* returnValue;
    __asm__ volatile ("mv %[rv], a0": [rv] "=r" (returnValue));

    return returnValue;
}



/*
void chhResume(){
    __asm__ volatile("csrw stvec, %[adrFun]": : [adrFun]"r" (&interruptHandler));
    //__asm__ volatile ("csrc sip, 0x02");
}
 */
/*
void Riscv::lock(unsigned* lck) {
    while(copy_and_swap(lck, 0, 1));
}

void Riscv::unlock(unsigned* lck) {
    while(copy_and_swap(lck, 1, 0));
}
 */

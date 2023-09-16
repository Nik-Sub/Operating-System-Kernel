//
// Created by os on 8/17/22.
//

#ifndef PROJECT_BASE_V1_1_SYSTEMCONSOLE_H
#define PROJECT_BASE_V1_1_SYSTEMCONSOLE_H
#include "MyBuffer.hpp"


class SystemConsole {
private:
    static SystemConsole* console;
public:
    SystemConsole(){
        // i had problem with initializing static fields, so i did this
        inputBuffer = new MyBuffer(256);
        outputBuffer = new MyBuffer(256);
        // both are 0 because we want to block console threads
        sem_open(&outputSem,0);
        sem_open(&extIntSemForOutput,0);

    }
    static sem_t outputSem;
    static sem_t extIntSemForOutput;
    static MyBuffer* inputBuffer;
    static MyBuffer* outputBuffer;

    static SystemConsole* getInstance(){
        if (console == nullptr)
            console = new SystemConsole();
        return console;
    }

    static void funForOutputThread(void* p){

        uint64 mask = 0x01;
        __asm__ volatile("csrc sstatus, %[par]": : [par]"r" (mask));
        while (true) {
            // if buffer is empty, then wait
            //sem_wait(outputSem);
            //sem_wait(extIntSemForOutput);


            if (outputBuffer->getCnt() != 0 || *((uint8 *) CONSOLE_STATUS) & (1 << 5)) {
                while (*((uint8 *) CONSOLE_STATUS) & (1 << 5)) {
                    if (outputBuffer->getCnt() == 0 || !(*((uint8 *) CONSOLE_STATUS) & (1 << 5)))
                        break;
                    uint8 *tranReg = (uint8 *) CONSOLE_TX_DATA;
                    uint8 forPrint = outputBuffer->get();

                    *tranReg = forPrint;
                }
            }
            thread_dispatch();

        }
    }

    static void busyWait(void* p){
        while(true){
            if (Scheduler::readyThreadQueue.peekFirst())
                thread_dispatch();
        }
    }
};




#endif //PROJECT_BASE_V1_1_SYSTEMCONSOLE_H

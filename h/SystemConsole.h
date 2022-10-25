//
// Created by os on 8/17/22.
//

#ifndef PROJECT_BASE_V1_1_SYSTEMCONSOLE_H
#define PROJECT_BASE_V1_1_SYSTEMCONSOLE_H
#include "MyBuffer.hpp"


class SystemConsole {
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

    static void funForOutputThread(void* p){
        while (true) {
            // if buffer is empty, then wait
            sem_wait(outputSem);
            sem_wait(extIntSemForOutput);
            while (*((uint8 *) CONSOLE_STATUS) & (1 << 5)) {
                uint8 *tranReg = (uint8 *) CONSOLE_TX_DATA;
                uint8 forPrint = SystemConsole::outputBuffer->get();

                *tranReg = forPrint;
            }
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

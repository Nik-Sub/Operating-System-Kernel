//
// Created by os on 8/18/22.
//

#ifndef PROJECT_BASE_V1_1_MYBUFFER_H
#define PROJECT_BASE_V1_1_MYBUFFER_H


#include "../h/syscall_c.h"
//#include "../test/printing.hpp"

class MyBuffer {
private:
    int cap;
    int *buffer;
    int head, tail;

    sem_t spaceAvailable;
    sem_t itemAvailable;
    sem_t mutexHead;
    sem_t mutexTail;

public:
    MyBuffer(int _cap);
    //MyBuffer(){
    //    MyBuffer(50);
    //}
    ~MyBuffer();

    void put(uint8 val);
    uint8 get();

    int getCnt();

};



#endif //PROJECT_BASE_V1_1_MYBUFFER_H

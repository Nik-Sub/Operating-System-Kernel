//
// Created by os on 8/18/22.
//

#include "../h/MyBuffer.hpp"

MyBuffer::MyBuffer(int _cap) : cap(_cap + 1), head(0), tail(0) {
    buffer = (int*)mem_alloc(sizeof(int) * cap);

    sem_open(&itemAvailable, 0);
    sem_open(&spaceAvailable, _cap);
    sem_open(&mutexHead, 1);
    sem_open(&mutexTail, 1);
}

MyBuffer::~MyBuffer() {
    /*
    __putc('\n');
    printString("Buffer deleted!\n");
    while (getCnt() > 0) {
        char ch = buffer[head];
        __putc(ch);
        head = (head + 1) % cap;
    }
    __putc('!');
    __putc('\n');
     */

    mem_free(buffer);
    sem_close(itemAvailable);
    sem_close(spaceAvailable);
    sem_close(mutexTail);
    sem_close(mutexHead);
}

void MyBuffer::put(uint8 val) {
    sem_wait(spaceAvailable);

    sem_wait(mutexTail);
    buffer[tail] = val;
    tail = (tail + 1) % cap;
    sem_signal(mutexTail);

    sem_signal(itemAvailable);

}

uint8 MyBuffer::get() {
    sem_wait(itemAvailable);

    sem_wait(mutexHead);

    uint ret = buffer[head];
    head = (head + 1) % cap;
    sem_signal(mutexHead);

    sem_signal(spaceAvailable);

    return ret;
}

int MyBuffer::getCnt() {
    //__asm__ volatile ("mv a0, %[var]": : [var]"r" (this));
    int ret;

    sem_wait(mutexHead);
    sem_wait(mutexTail);

    if (tail >= head) {
        ret = tail - head;
    } else {
        ret = cap - head + tail;
    }

    sem_signal(mutexTail);
    sem_signal(mutexHead);

    return ret;
}

void *MyBuffer::operator new(size_t s) {
    return SlabAllocator::getInstance()->alloc("CacheBuffersConsole", s);
}

void MyBuffer::operator delete(void *p) {
    if (p == nullptr)
        return;
    slabMemFree("CacheBuffersConsole", p);
}

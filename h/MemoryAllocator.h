//
// Created by os on 6/23/22.
//

#ifndef PROJECT_BASE_MEMORYALLOCATOR_H
#define PROJECT_BASE_MEMORYALLOCATOR_H

#include "../lib/hw.h"

struct FreeMemNode{
    // size in blocks
    size_t size;
    FreeMemNode* next;
    FreeMemNode* prev;
};

struct NotFreeMemNode{
    void* addr;
    // size in blocks
    size_t size;
    NotFreeMemNode* next;
    NotFreeMemNode* prev;
};

class MemoryAllocator {
    // we will make memAl on the start of the heap
    static MemoryAllocator* memAl;
    static FreeMemNode* head;
    static NotFreeMemNode* notFreeHead;
    MemoryAllocator();
    int tryToJoin(FreeMemNode*);
public:
    static MemoryAllocator* getInstance();

    // size is number of blocks, not number of bytes
    void* memAlloc(size_t size);
    int freeMem(void* addr);




};


#endif //PROJECT_BASE_MEMORYALLOCATOR_H

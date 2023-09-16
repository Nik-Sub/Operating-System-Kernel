//
// Created by os on 2/2/23.
//

#ifndef POSLVERZIJAZAPREDAJUKONACNA4_BUDDYALLOCATOR_H
#define POSLVERZIJAZAPREDAJUKONACNA4_BUDDYALLOCATOR_H

#include "../lib/hw.h"
struct FreeMemBlockBuddy{
    void* addr;
    struct FreeMemBlockBuddy* next;
};

class BuddyAllocator {
    static BuddyAllocator* buddyAllocator;
    // start of memory for kernel, will need that to merge blocks
    void* memoryStart;
    size_t sizeOfKernelMemoryInBlocks;
    static size_t maxDegree;
    FreeMemBlockBuddy** arrayOfFreeBlocks;

public:
    static BuddyAllocator* getInstance();

//private:
    // sizeInBlocks will be 2^n
    void* buddyMemAlloc(size_t sizeInBlocks);
    void buddyMemFree(void* addr, size_t sizeInBlocks);
    int tryToMerge(void*addr, size_t degree, size_t numOfBlocks);
};


#endif //POSLVERZIJAZAPREDAJUKONACNA4_BUDDYALLOCATOR_H

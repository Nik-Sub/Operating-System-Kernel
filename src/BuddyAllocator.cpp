//
// Created by os on 2/2/23.
//

#include "../h/BuddyAllocator.h"
#include "../h/Slab.h"

BuddyAllocator* BuddyAllocator::buddyAllocator = nullptr;
size_t BuddyAllocator::maxDegree = 0;

BuddyAllocator *BuddyAllocator::getInstance() {
    if (buddyAllocator == nullptr){
        buddyAllocator = (BuddyAllocator*)HEAP_START_ADDR;
        //size in blocks for BuddyAllocator
        size_t sizeInBlocksBuddy = 0;
        if (sizeof(BuddyAllocator))
            sizeInBlocksBuddy = sizeof(BuddyAllocator) / BLOCK_SIZE + (sizeof(BuddyAllocator) % BLOCK_SIZE == 0? 0: 1);

        //size in blocks for KernelMemory
        size_t sizeOfKernelMemory =  ((size_t)((uint8 *)HEAP_END_ADDR - (uint8 *)HEAP_START_ADDR)/ 8);
        size_t sizeOfKernelMemoryInBlocks = sizeOfKernelMemory / BLOCK_SIZE + (sizeOfKernelMemory % BLOCK_SIZE == 0? 0: 1);
        // round on power of 2
        size_t power = 1;
        size_t degree = 0;
        while (power * 2 <= sizeOfKernelMemoryInBlocks) {
            power *= 2;
            degree++;
        }
        if (power < sizeOfKernelMemoryInBlocks) {
            sizeOfKernelMemoryInBlocks = power * 2;
            degree++;
        }

        buddyAllocator->maxDegree = degree;
        buddyAllocator->arrayOfFreeBlocks = (FreeMemBlockBuddy**)((uint8* )HEAP_START_ADDR + sizeInBlocksBuddy * BLOCK_SIZE);
        //initialization of array
        for (size_t i = 0; i < degree; i++)
            buddyAllocator->arrayOfFreeBlocks[i] = nullptr;
        buddyAllocator->arrayOfFreeBlocks[degree] = (FreeMemBlockBuddy*)((uint8*)HEAP_START_ADDR + sizeInBlocksBuddy * BLOCK_SIZE + (degree + 1) * sizeof(FreeMemBlockBuddy*));
        buddyAllocator->arrayOfFreeBlocks[degree]->addr = buddyAllocator->arrayOfFreeBlocks[degree];
        buddyAllocator->arrayOfFreeBlocks[degree]->next = nullptr;
        buddyAllocator->sizeOfKernelMemoryInBlocks = sizeOfKernelMemoryInBlocks;
        // start of memory for kernel
        buddyAllocator->memoryStart = buddyAllocator->arrayOfFreeBlocks[degree]->addr;

    }
    return buddyAllocator;
}

void *BuddyAllocator::buddyMemAlloc(size_t sizeInBlocks) {
    // first we need to find degree of 2 for sizeInBlocks
    size_t power = 1;
    size_t degree = 0;
    while (power * 2 <= sizeInBlocks) {
        power *= 2;
        degree++;
    }
    if (power < sizeInBlocks) {
        sizeInBlocks = power * 2;
        degree++;
    }
    FreeMemBlockBuddy* ret = nullptr;
    if (arrayOfFreeBlocks[degree] != nullptr){
        ret = arrayOfFreeBlocks[degree];
        arrayOfFreeBlocks[degree] = ret->next;
    }
    // we will need to check bigger blocks and divide them
    else{
        size_t startDegree = degree;
        while(++degree <= maxDegree && arrayOfFreeBlocks[degree] == nullptr);
        // ther is not free memory
        if (degree > maxDegree)
            return ret;
        ret = arrayOfFreeBlocks[degree];
        // moving ptr to another block of that size
        arrayOfFreeBlocks[degree] = ret->next;

        // now adding other half of block to the ptrs that points to the blocks with lower size
        // we will return left part
        while(degree > startDegree) {
            size_t newSizeInBlocks = 1;
            for (size_t i = 0; i < degree; i++)
                newSizeInBlocks *= 2;
            void *otherHalf = (uint8 *) ret + newSizeInBlocks / 2 * BLOCK_SIZE;
            arrayOfFreeBlocks[degree - 1] = (FreeMemBlockBuddy*)otherHalf;
            arrayOfFreeBlocks[degree - 1]->addr = arrayOfFreeBlocks[degree - 1];
            arrayOfFreeBlocks[degree - 1]->next = nullptr;
            degree--;
        }
    }
    return ret;

}

void BuddyAllocator::buddyMemFree(void *addr, size_t sizeInBlocks) {
    // looking for degree
    size_t power = 1;
    size_t degree = 0;
    while (power * 2 <= sizeInBlocks) {
        power *= 2;
        degree++;
    }
    if (power < sizeInBlocks) {
        sizeInBlocks = power * 2;
        degree++;
    }
    FreeMemBlockBuddy* oldFirst = arrayOfFreeBlocks[degree];
    arrayOfFreeBlocks[degree] = (FreeMemBlockBuddy*)addr;
    arrayOfFreeBlocks[degree]->addr = addr;
    arrayOfFreeBlocks[degree]->next = oldFirst;

    //try to merge
    tryToMerge(addr, degree, sizeInBlocks);
}

int BuddyAllocator::tryToMerge(void *addr, size_t degree, size_t numOfBlocks) {
    uint8* curAddr = (uint8*)memoryStart;

    // left - parity is true, right nore - pariy is false
    bool parity = true;
    uint8* buddy = nullptr;
    while (true){
        if (curAddr == addr) {
            break;
        }
        if (parity == true)
            parity = false;
        else
            parity = true;
        curAddr += numOfBlocks * BLOCK_SIZE;
    }
    // defining buddy
    if (parity == true)
        buddy = curAddr +  numOfBlocks * BLOCK_SIZE;
    else
        buddy = curAddr - numOfBlocks * BLOCK_SIZE;

    FreeMemBlockBuddy* cur = arrayOfFreeBlocks[degree];
    FreeMemBlockBuddy* pred = nullptr;
    while (cur){
        if (cur->addr == buddy){
            // if cur is not first node
            if (pred != nullptr){
                pred->next = cur->next;
            }
            break;
        }
        pred = cur;
        cur = cur->next;
    }

    if (buddy == nullptr)
        return 1;
    else{
        // finding lower address, that will be start address for bigger degree
        void* leftAddress = (buddy > addr? addr: buddy);
        if (degree < maxDegree)
            tryToMerge(leftAddress,degree+1,numOfBlocks*2);
        return 0;
    }

}



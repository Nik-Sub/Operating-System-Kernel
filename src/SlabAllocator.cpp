//
// Created by os on 2/2/23.
//

#include "../h/SlabAllocator.h"
#include "../h/tcb.h"
#include "../h/scb.hpp"
#include "../h/Slab.h"

SlabAllocator* SlabAllocator::instance = nullptr;

SlabAllocator::SlabAllocator() {
    cacheHead = nullptr;
    /*
    Cache* cur = cacheHead = new Cache("CacheTCB",sizeof(TCB), nullptr, nullptr);
    cur->prev = nullptr;
    Cache* pred = cur;
    cur = cur->next = new Cache("CacheSCB", sizeof(SCB), nullptr, nullptr);
    cur->prev = pred;
    pred = cur;
    cur = cur->next = new Cache("CacheBuffersConsole", 0b100000000, nullptr, nullptr);
    cur->prev = pred;
    pred = cur;
    cur = cur->next = new Cache("CacheThreadStacks", DEFAULT_STACK_SIZE, nullptr, nullptr);
    cur->prev = pred;
    cacheTail = cur;
     */
}

void *SlabAllocator::operator new(size_t s) {
    size_t sizeInBlocks = s / BLOCK_SIZE + (s % BLOCK_SIZE == 0? 0: 1);
    return BuddyAllocator::getInstance()->buddyMemAlloc(sizeInBlocks);
}

void SlabAllocator::operator delete(void *p) {
    size_t sizeInBlocks = sizeof(SlabAllocator) / BLOCK_SIZE + (sizeof(SlabAllocator) % BLOCK_SIZE == 0? 0: 1);
    BuddyAllocator::getInstance()->buddyMemFree(p,sizeInBlocks);
}

SlabAllocator::~SlabAllocator() {
    Cache* cur = cacheHead;
    while (cur){
        delete cur;
        cur = cur->next;
    }
}

SlabAllocator *SlabAllocator::getInstance() {
    if (instance == nullptr)
        instance = new SlabAllocator();
    return instance;
}

void *SlabAllocator::alloc(const char *name, size_t sizeOfData) {
    Cache *cur = cacheHead;
    while (cur != nullptr) {
        // if we want to alloc small buffer we will look size of data of cache, not at name
        if(strComp(name, "")){
            if (cur->sizeOfData == sizeOfData)
                break;
        }
        else{
            // if names are the same, we got right cache
            if (strComp(cur->getName(), name)){
                break;
            }
        }

        cur = cur->next;
    }
    // makes new cache and put him in list
    if (cur == nullptr) {
        cur = new Cache(name, sizeOfData, nullptr, nullptr);
        Cache* oldHead = cacheHead;
        cacheHead = cur;
        cur->next = oldHead;
        if (oldHead != nullptr)
            oldHead->prev = cur;
    }
    void *retAddr = kmem_cache_alloc(cur);
    return retAddr;
}

void SlabAllocator::free(char const* name, const void * p) {
    Cache *cur = cacheHead;
    while (true) {
        // if we want to free small buffer we will look size of data of cache, not at name
        if (strComp(name, "")){
            bool findInHalfFull = false;
            bool findInFull = false;
            // first we will look at halfFull slabs
            Cache::Slab *curSlab = cur->halfFull;
            while (curSlab != nullptr) {

                if (curSlab->mem <= p && p <= (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cur->sizeOfData)) {
                    findInHalfFull = true;
                    break;
                }


                curSlab = curSlab->next;
            }
            // if we did not find, we will lok in full slabs
            if (findInHalfFull == false) {
                curSlab = cur->full;
                while (curSlab != nullptr) {

                    //void* end = (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cur->sizeOfData);
                    if (curSlab->mem <= p && p <= (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cur->sizeOfData)) {
                        findInFull = true;
                        break;
                    }

                    curSlab = curSlab->next;
                }
            }
            if (findInHalfFull || findInFull){
                // now we found slab with that address
                curSlab->setFreeSlot(p);

                //now move it from full to halffull if we found slab there
                if (findInFull) {
                    Cache::Slab *pred = curSlab->prev;
                    Cache::Slab *next = curSlab->next;
                    // move full to next halFull
                    if (cur->full == curSlab)
                        cur->full = next;

                    if (pred)
                        pred->next = next;
                    if (next)
                        next->prev = pred;
                    curSlab->next = nullptr;
                    curSlab->prev = nullptr;
                    // now put new halfFull in list of halfFull slabs
                    if (cur->halfFull == nullptr) {
                        cur->halfFull = curSlab;
                    }
                        // we will put slab on 2nd place
                    else {
                        Cache::Slab *nextHalfFull = cur->halfFull->next;
                        cur->halfFull->next = curSlab;
                        curSlab->prev = cur->halfFull;
                        curSlab->next = nextHalfFull;
                        if (nextHalfFull)
                            nextHalfFull->prev = curSlab;
                    }
                }

                //if halffull become empty, delete that slab
                if (findInHalfFull) {
                    if (curSlab->cnt == 0) {
                        Cache::Slab *pred = curSlab->prev;
                        Cache::Slab *next = curSlab->next;
                        // move halfFull to next halFull
                        if (cur->halfFull == curSlab)
                            cur->halfFull = next;
                        if (pred)
                            pred->next = next;
                        if (next)
                            next->prev = pred;


                        delete curSlab;
                    }
                }
                break;
            }
        }
        else{
            if (strComp(cur->getName(), name)){
                break;
            }
        }
        cur = cur->next;
    }

    kmem_cache_free(cur, p);

}


bool strComp(char const* s1, char const* s2){
    while (*s1 != '\0' && *s2 != '\0'){
        if (*s1 == *s2) {
            s1++;
            s2++;
        }
        else
            break;
    }
    if (*s1 != *s2)
        return false;
    return true;
}

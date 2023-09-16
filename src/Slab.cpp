//
// Created by os on 2/7/23.
#include "../h/Slab.h"
#include "../h/SlabAllocator.h"
#include "../h/printing.hpp"



void kmem_init(void *space, int block_num){
    // when we initialize slab allocator buddy allocator will be initialized too
    SlabAllocator::getInstance();
}

kmem_cache_t *kmem_cache_create(const char *name, size_t size,
                                void (*ctor)(void *),
                                void (*dtor)(void *))
{
    Cache* ret = new Cache(name,size, ctor, dtor);
    return ret;
}

int kmem_cache_shrink(kmem_cache_t *cachep) {
    Cache::Slab* curSlab = cachep->empty;
    while (curSlab != nullptr) {
        Cache::Slab *next = curSlab->next;
        delete curSlab;
        curSlab = next;
    }
    cachep->empty = nullptr;
    return 0;
}
void *kmem_cache_alloc(kmem_cache_t *cachep) {
    void *retAddr;
    // all halfFull slabs are the same, we can use slot from any of them, so i will look at first
    if (cachep->halfFull != nullptr) {
        retAddr = cachep->halfFull->getFreeSlot();
        // if halfFull becomes full
        if (cachep->halfFull->cnt == 0) {
            Cache::Slab *curSlab = cachep->halfFull;
            Cache::Slab *pred = curSlab->prev;
            Cache::Slab *next = curSlab->next;
            // move halfFull to next halFull
            cachep->halfFull = next;

            if (pred)
                pred->next = next;
            if (next)
                next->prev = pred;
            curSlab->next = nullptr;
            curSlab->prev = nullptr;
            // now put new full in list of full slabs
            if (cachep->full == nullptr) {
                cachep->full = curSlab;
            }
                // we will put slab on 2nd place
            else {
                Cache::Slab *nextFull = cachep->full->next;
                cachep->full->next = curSlab;
                curSlab->prev = cachep->full;
                curSlab->next = nextFull;
                if (nextFull)
                    nextFull->prev = curSlab;
            }
        }
    }
        // now look at empty slabs
    else {
        // if we dont have empty slabs we will automatically make one
        if (cachep->empty == nullptr) {
            cachep->empty = new Cache::Slab(cachep->sizeOfData, cachep);
            if (cachep->empty == nullptr) {
                cachep->error = true;
                // it means we do not have free space
                return nullptr;
            }
        }
        retAddr = cachep->empty->getFreeSlot();
        // move that empty to halfFull slots
        Cache::Slab *curSlab = cachep->empty;
        Cache::Slab *pred = curSlab->prev;
        Cache::Slab *next = curSlab->next;
        // move empty to next empty
        cachep->empty = next;

        if (pred)
            pred->next = next;
        if (next)
            next->prev = pred;
        curSlab->next = nullptr;
        curSlab->prev = nullptr;
        // now put new halfFull in list of halfFull slabs
        if (cachep->halfFull == nullptr) {
            cachep->halfFull = curSlab;
        }
            // we will put slab on 2nd place
        else {
            Cache::Slab *nextHalfFull = cachep->halfFull->next;
            cachep->halfFull->next = curSlab;
            curSlab->prev = cachep->full;
            curSlab->next = nextHalfFull;
            if (nextHalfFull)
                nextHalfFull->prev = curSlab;
        }
    }

    return retAddr;
}
void kmem_cache_free(kmem_cache_t *cachep, const void *objp){
    bool findInHalfFull = false;
    bool findInFull = false;
    // first we will look at halfFull slabs
    Cache::Slab *curSlab = cachep->halfFull;
    while (curSlab != nullptr) {

        if (curSlab->mem <= objp && objp <= (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cachep->sizeOfData)) {
            findInHalfFull = true;
            break;
        }


        curSlab = curSlab->next;
    }
    // if we did not find, we will lok in full slabs
    // we will always find here if we did not at halfFull
    if (findInHalfFull == false) {
        curSlab = cachep->full;
        while (curSlab != nullptr) {

            //void* end = (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cur->sizeOfData);
            if (curSlab->mem <= objp && objp <= (void *) ((uint8 *) curSlab->mem + SLAB_SIZE * cachep->sizeOfData)) {
                findInFull = true;
                break;
            }

            curSlab = curSlab->next;
        }
    }
    // now we found slab with that address
    curSlab->setFreeSlot(objp);

    //now move it from full to halffull if we found slab there
    if (findInFull) {
        Cache::Slab *pred = curSlab->prev;
        Cache::Slab *next = curSlab->next;
        // move full to next halFull
        if (cachep->full == curSlab)
            cachep->full = next;

        if (pred)
            pred->next = next;
        if (next)
            next->prev = pred;
        curSlab->next = nullptr;
        curSlab->prev = nullptr;
        // now put new halfFull in list of halfFull slabs
        if (cachep->halfFull == nullptr) {
            cachep->halfFull = curSlab;
        }
            // we will put slab on 2nd place
        else {
            Cache::Slab *nextHalfFull = cachep->halfFull->next;
            cachep->halfFull->next = curSlab;
            curSlab->prev = cachep->halfFull;
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
            if (cachep->halfFull == curSlab)
                cachep->halfFull = next;
            if (pred)
                pred->next = next;
            if (next)
                next->prev = pred;


            delete curSlab;
        }
    }
}
void *kmalloc(size_t size){
    return SlabAllocator::getInstance()->alloc("", size);
}
void kfree(const void *objp){
    SlabAllocator::getInstance()->free("", objp);
}
void kmem_cache_destroy(kmem_cache_t *cachep){
    delete cachep;
}
void kmem_cache_info(kmem_cache_t *cachep){
    printString("INFO CACHE:\n");
    printString("Name: ");
    printString(cachep->getName());
    printString("\n");
    printString("Size of data: ");
    printInt(cachep->sizeOfData);
    printString("\n");
    printString("Number of blocks in cache: ");
    //size in blocks for one slab
    size_t sizeInBlocks = cachep->sizeOfData * SLAB_SIZE / BLOCK_SIZE + (cachep->sizeOfData * SLAB_SIZE % BLOCK_SIZE == 0? 0: 1);;
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
    printInt(cachep->numOfSlabs * sizeInBlocks);
    printString("\n");
    printString("Number of slabs: ");
    printInt(cachep->numOfSlabs);
    printString("\n");
    printString("Number of object in one slab: ");
    printInt(SLAB_SIZE);
    printString("\n");
    printString("Occupancy: ");
    printInt(cachep->numOfObjectsInCache * 100 / (cachep->numOfSlabs * SLAB_SIZE) );
    printString("%");
    printString("\n");
    printString("\n");

}
int kmem_cache_error(kmem_cache_t *cachep){
    if (cachep->error)
        return 1;
    return 0;
}

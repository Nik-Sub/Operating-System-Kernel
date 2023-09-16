//
// Created by os on 2/6/23.
//

#include "../h/Cache.h"
#include "../h/Slab.h"

Cache::Cache(char const *name, size_t sizeOfData, Body2 ctor, Body2 dtor) {
    this->name = name;
    this->sizeOfData = sizeOfData;
    this->ctor = ctor;
    this->dtor = dtor;
    full = nullptr;
    halfFull = nullptr;
    empty = new Slab(sizeOfData, this);
    if (empty == nullptr)
        error = true;
    next = nullptr;
    prev = nullptr;

}



Cache::~Cache() {


    if (full != nullptr) {
        Cache::Slab* cur = full;
        while (cur != nullptr) {
            Cache::Slab* next = cur->next;
            delete cur;
            cur = next;
        }
        full = nullptr;
    }
    if (halfFull != nullptr) {
        Cache::Slab* cur = halfFull;
        while (cur != nullptr) {
            Cache::Slab* next = cur->next;
            delete cur;
            cur = next;
        }
        halfFull = nullptr;
    }
    if (empty != nullptr){
        Cache::Slab* cur = empty;
        while (cur != nullptr) {
            Cache::Slab* next = cur->next;
            delete cur;
            cur = next;
        }
        empty = nullptr;
    }

}

void *Cache::operator new(size_t s) {
    size_t sizeInBlocks = s / BLOCK_SIZE + (s % BLOCK_SIZE == 0? 0: 1);
    return BuddyAllocator::getInstance()->buddyMemAlloc(sizeInBlocks);
}

void Cache::operator delete(void *p) {
    size_t sizeInBlocks = sizeof(Cache) / BLOCK_SIZE + (sizeof(Cache) % BLOCK_SIZE == 0? 0: 1);
    BuddyAllocator::getInstance()->buddyMemFree(p,sizeInBlocks);
}



Cache::Slab::Slab(size_t sizeOfData, Cache* owner) {
    this->owner = owner;
    this->sizeOfData = sizeOfData;
    prev = nullptr;
    next = nullptr;
    // we will always make SLAB_SIZE slots for every type of object
    size_t sizeOfArrayForSlab = SLAB_SIZE * sizeOfData ;
    size_t sizeInBlocks = sizeOfArrayForSlab / BLOCK_SIZE + (sizeOfArrayForSlab % BLOCK_SIZE == 0? 0: 1);
    // this will be void* and when we want to allocate one slot we will cast it relative to name of cache
    mem = BuddyAllocator::getInstance()->buddyMemAlloc(sizeInBlocks);
    if (owner->ctor != nullptr)
        for (size_t i = 0; i < SLAB_SIZE; i++)
            owner->ctor((uint8*)mem + i * owner->sizeOfData);
    //SLAB_SIZE flags to know if slot is free
    sizeInBlocks = SLAB_SIZE * sizeof(uint8) / BLOCK_SIZE + (SLAB_SIZE * sizeof(uint8) % BLOCK_SIZE == 0? 0: 1);
    freeVector = (uint8*)BuddyAllocator::getInstance()->buddyMemAlloc(sizeInBlocks);
    for (int i = 0; i < SLAB_SIZE; i++)
        // to set it free
        freeVector[i] = 0;

    owner->numOfSlabs++;
}

Cache::Slab::~Slab() {
// size in blocks for Slabs array
    size_t sizeOfArrayForSlab = SLAB_SIZE * sizeOfData ;
    size_t sizeInBlocksArrayForSlab = sizeOfArrayForSlab / BLOCK_SIZE + (sizeOfArrayForSlab % BLOCK_SIZE == 0? 0: 1);
    // size in blocks for array of flags
    //SLAB_SIZE flags to know if slot is free
    size_t sizeInBlocksFlags = SLAB_SIZE * sizeof(uint8) / BLOCK_SIZE + (SLAB_SIZE * sizeof(uint8) % BLOCK_SIZE == 0? 0: 1);

    BuddyAllocator::getInstance()->buddyMemFree(mem, sizeInBlocksArrayForSlab);
    BuddyAllocator::getInstance()->buddyMemFree(freeVector, sizeInBlocksFlags);
    owner->numOfSlabs--;
}


void *Cache::Slab::operator new(size_t s) {
    size_t sizeInBlocks = s / BLOCK_SIZE + (s % BLOCK_SIZE == 0? 0: 1);
    return BuddyAllocator::getInstance()->buddyMemAlloc(sizeInBlocks);
}

void Cache::Slab::operator delete(void *p) {
    size_t sizeInBlocks = sizeof(Slab) / BLOCK_SIZE + (sizeof(Slab) % BLOCK_SIZE == 0? 0: 1);
    BuddyAllocator::getInstance()->buddyMemFree(p,sizeInBlocks);
}

void *Cache::Slab::getFreeSlot() {
    if (mem == nullptr){
        return nullptr;
    }
    if (cnt == 0)
        return nullptr;
    for (int i = 0; i < SLAB_SIZE; i++){
        if (freeVector[i] == 0) {
            freeVector[i] = 1;
            cnt--;
            owner->numOfObjectsInCache++;
            return (uint8*)mem + sizeOfData * i;
        }
    }
    owner->error = true;
    return nullptr;
}

void Cache::Slab::setFreeSlot(const void *a) {
    size_t ind = (uint8*)a - (uint8*)mem;
    ind = ind / sizeOfData;
    freeVector[ind] = 0;
    cnt++;
    owner->numOfObjectsInCache--;
}



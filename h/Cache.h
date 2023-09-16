//
// Created by os on 2/6/23.
//

#ifndef POSLVERZIJAZAPREDAJUKONACNA4_CACHE_H
#define POSLVERZIJAZAPREDAJUKONACNA4_CACHE_H
#include "BuddyAllocator.h"

#define SLAB_SIZE 64

class SlabAllocator;


class Cache {

    friend class SlabAllocator;
public:
    struct Slab {
        Cache* owner;
        size_t sizeOfData;
        Slab *next, *prev;
        void* mem;
        // to know which slot is free
        uint8* freeVector;
        // how many FREE slots slab has
        int cnt = SLAB_SIZE;
        // sizeOfData is in bytes
        Slab(size_t sizeOfData, Cache* owner);
        void* operator new (size_t s);
        void operator delete (void* p);
        ~Slab();

        // return address of free slot for that slab
        void* getFreeSlot();
        void setFreeSlot(const void* a);


    };

    Slab* full;
    Slab* halfFull;
    Slab* empty;
    // sizeOfData in bytes
    size_t sizeOfData;
    size_t numOfSlabs = 0;
    size_t numOfObjectsInCache = 0;
    bool error = false;
private:
    char const * name;
    Cache* next;
    Cache* prev;

    using Body2 = void (*)(void*);

    Body2 ctor;
    Body2 dtor;


public:
    Cache(char const * name, size_t sizeOfData, Body2 ctor, Body2 dtor);
    ~Cache();
    void* operator new (size_t s);
    void operator delete (void* p);

    char const* getName(){
        return name;
    }


};


#endif //POSLVERZIJAZAPREDAJUKONACNA4_CACHE_H

//
// Created by os on 2/2/23.
//

#ifndef POSLVERZIJAZAPREDAJUKONACNA4_SLABALLOCATOR_H
#define POSLVERZIJAZAPREDAJUKONACNA4_SLABALLOCATOR_H

#include "Cache.h"

bool strComp(char const* s1, char const* s2);

class SlabAllocator {
    public:
        SlabAllocator ();
        // name of cache
        void* alloc (char const * name, size_t sizeOfData);
        void free (char const* name, const void*);


        static SlabAllocator* getInstance();

        void* operator new (size_t s);
        void operator delete (void* p);
        ~SlabAllocator();
    private:
        static SlabAllocator* instance;

        Cache *cacheHead, *cacheTail;
        //size_t slabSize;


};



#endif //POSLVERZIJAZAPREDAJUKONACNA4_SLABALLOCATOR_H

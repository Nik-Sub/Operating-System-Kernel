#include "../h/MemoryAllocator.h"
#include "../h/syscall_c.h"

MemoryAllocator* MemoryAllocator::memAl = nullptr;
FreeMemNode* MemoryAllocator::head = nullptr;
NotFreeMemNode* MemoryAllocator::notFreeHead = nullptr;


MemoryAllocator *MemoryAllocator::getInstance() {
    if (!memAl) {
        memAl = (MemoryAllocator *) HEAP_START_ADDR;
        // size in blocks for memAl
        size_t sizeInBlocks = 0;
        if (sizeof(MemoryAllocator))
            sizeInBlocks = sizeof(MemoryAllocator) / MEM_BLOCK_SIZE + (sizeof(MemoryAllocator) % MEM_BLOCK_SIZE == 0? 0: 1);
        head = (FreeMemNode *) ((char *) HEAP_START_ADDR + sizeInBlocks * MEM_BLOCK_SIZE );
        head->next = nullptr;
        head->prev = nullptr;
        head->size = (size_t )(((char*)HEAP_END_ADDR - (char*)head)/ MEM_BLOCK_SIZE + (((char*)HEAP_END_ADDR - (char*)head)% MEM_BLOCK_SIZE == 0? 0: 1));
    }
    return memAl;
}

void *MemoryAllocator::memAlloc(size_t size) {
    FreeMemNode* prev = nullptr;
    FreeMemNode* cur = head;
    void* ret = nullptr;
    // i will need this size to find NotFreeMemNode
    size_t oldSize = size;
    while (cur){
        // size in blocks of FreeMemNode
        size_t sizeInBlocksFM = sizeof(FreeMemNode) / MEM_BLOCK_SIZE + (sizeof(FreeMemNode) % MEM_BLOCK_SIZE == 0? 0: 1);
        // size in blocks of NotFreeMemNode
        size_t sizeInBlocksNFM = sizeof(NotFreeMemNode) / MEM_BLOCK_SIZE + (sizeof(NotFreeMemNode) % MEM_BLOCK_SIZE == 0? 0: 1);
        if (cur->size - sizeInBlocksFM >= size + sizeInBlocksNFM ){
            size += sizeInBlocksNFM;
            ret = cur;
            if (prev != nullptr){
                if(cur->size == size){
                    prev->next = cur->next;
                    if (prev->next)
                        prev->next->prev = prev;
                }
                else{
                    size_t newSize = cur->size - size;
                    prev->next = (FreeMemNode*)((char*)cur + size * MEM_BLOCK_SIZE);
                    if ( cur->next ) {
                        cur->next->prev = prev->next;
                    }
                    FreeMemNode* temp = cur->next;
                    cur = prev->next;
                    cur->prev = prev;
                    cur->next = temp;
                    cur->size = newSize;
                }
            }
            // it's first node
            else{
                if(cur->size == size){
                    head = head->next;
                    head->prev = nullptr;
                }
                else{
                    size_t newSize = cur->size - size;
                    FreeMemNode* temp = cur->next;
                    //FreeMemNode* temp2 = (FreeMemNode*)((char*)cur + size * MEM_BLOCK_SIZE);
                    cur = (FreeMemNode*)((char*)cur + size * MEM_BLOCK_SIZE);
                    if (temp)
                        temp->prev = cur;
                    cur->next = temp;
                    cur->prev = nullptr;
                    cur->size = newSize;
                    head = cur;
                }
            }

            // adding taken memory to the list
            // NotFreeMemNode is right after taken memory
            NotFreeMemNode* newTemp = (NotFreeMemNode*)((char*)ret + oldSize*MEM_BLOCK_SIZE);

            newTemp->size = oldSize;// without memory for NotFreeMemNode
            newTemp->next = newTemp->prev = nullptr;
            newTemp->addr = ret;
            if (notFreeHead == nullptr)
                notFreeHead = newTemp;
            else{
                NotFreeMemNode* prev = nullptr;
                NotFreeMemNode* temp = notFreeHead;
                while(temp){
                    if (newTemp->addr < temp->addr){
                        break;
                    }
                    prev = temp;
                    temp = temp->next;
                }
                if (!temp){
                    prev->next = newTemp;
                    newTemp->prev = prev;
                }
                else{
                    newTemp->next = temp;
                    newTemp->prev = temp->prev;
                    temp->prev->next = newTemp;
                    temp->prev = newTemp;
                }
            }

            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if (ret == nullptr)
        head = nullptr;
    return ret;
}

int MemoryAllocator::freeMem(void *addr) {
    // Find the place where to insert the new free segment:
    // head is pointing to the first free part of memory
    FreeMemNode* cur=0;
    if (!head || addr<(char*)head)
        cur = 0; // insert as the firstu
    else
        for (cur=head; cur->next!=0 && addr>(char*)(cur->next);
             cur=cur->next);
    // Insert the new segment after cur:
    // size of segment which will be free ( + FreeMemNode !!!!! )
    NotFreeMemNode* temp = notFreeHead;
    // looking for NotFreeMemNode with addr address
    while (temp){
        if (temp->addr == addr)
            break;
        else
            temp = temp->next;
    }
    if (temp == nullptr)
        return -2;
    size_t size = temp->size + sizeof(NotFreeMemNode) / MEM_BLOCK_SIZE + (sizeof(NotFreeMemNode) % MEM_BLOCK_SIZE == 0? 0: 1);
    // deleting this NotFreeMemNode from list
    NotFreeMemNode* next = temp->next;
    NotFreeMemNode* prev = temp->prev;
    if (prev){
        prev->next = next;
    }
    if (next){
        next->prev = prev;
    }
    if (!prev && !next)
        notFreeHead = nullptr;

    FreeMemNode* newSeg = (FreeMemNode*)addr;
    newSeg->size = size;
    newSeg->prev = cur;
    if (cur) newSeg->next = cur->next;
    else newSeg->next = head;
    if (newSeg->next) newSeg->next->prev = newSeg;
    if (cur) cur->next = newSeg;
    else head = newSeg;
    // Try to merge with the previous and next segments:
    // (merging left node with right if it is possible
    tryToJoin(newSeg);
    tryToJoin(cur);
    return 0;

}

int MemoryAllocator::tryToJoin(FreeMemNode * cur) {
    if (!cur) return 0;
    if (cur->next && (char*)cur+cur->size*MEM_BLOCK_SIZE == (char*)(cur->next)) {
        // Remove the cur->next segment:
        cur->size += cur->next->size;
        cur->next = cur->next->next;
        if (cur->next) cur->next->prev = cur;
        if (cur->prev == nullptr)
            head = cur;
        return 1;
    } else
        return 0;

}

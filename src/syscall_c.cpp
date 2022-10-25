#include "../h/syscall_c.h"
//#include "../test/printing.hpp"


//extern "C" void* mem_allocS(size_t sizeInBlocks);
void* mem_alloc (size_t size){
    //__putc('U');
    size_t sizeInBlocks = size / MEM_BLOCK_SIZE  + (size % MEM_BLOCK_SIZE == 0? 0: 1);
    Riscv::forEveryFunc(0x01, &sizeInBlocks);
    void* addr;
    __asm__ volatile ("mv %[var], a0": [var]"=r" (addr));
    //__putc('I');
    return addr;
}



//extern "C" void* mem_freeS();
int mem_free (void* mem){
    void* retVal = Riscv::forEveryFunc(0x02, mem);
    //__putc('O');
    if (retVal != nullptr){
        return 0;
    }
    else{
        return -1;
    }
}


int thread_create (
        thread_t* handle,
        void(*start_routine)(void*),
        void* arg
){


    // push arg on stack beacuse we need code of func in a0
    __asm__ volatile ("addi sp, sp, -24");
    __asm__ volatile ("sd a0, 0x00(sp)");
    __asm__ volatile ("sd a1, 0x08(sp)");
    __asm__ volatile ("sd a2, 0x10(sp)");

    // postavljanje parametara
    // we are passing last location of stack
    __asm__ volatile ("mv a4, %[param]": :[param] "r" ((uint64*) mem_alloc(DEFAULT_STACK_SIZE * sizeof(uint64)) + DEFAULT_STACK_SIZE));
    __asm__ volatile ("mv a0, %[code]": :[code] "r" (0x11));
    __asm__ volatile ("ld a1, 0x00(sp)");
    __asm__ volatile ("ld a2, 0x08(sp)");
    __asm__ volatile ("ld a3, 0x10(sp)");

    __asm__ volatile ("addi sp, sp, 24");
    //uint64* stack = new uint64(DEFAULT_STACK_SIZE);



    __asm__ volatile ("ecall");

    if (*handle != nullptr)
        return 0;
    else
        return -1;

}

void thread_dispatch (){
    Riscv::forEveryFunc(0x13, nullptr);
}

int thread_exit (){
    Riscv::forEveryFunc(0x12, nullptr);
    void* retVal;
    __asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    // if we come here, its unsuccessfull
    return -1;
}

int sem_open (
        sem_t* handle,
        unsigned init
){

    // push arg on stack beacuse we need code of func in a0
    __asm__ volatile ("addi sp, sp, -16");
    __asm__ volatile ("sd a0, 0x00(sp)");
    __asm__ volatile ("sd a1, 0x08(sp)");

    // put args in registers
    __asm__ volatile ("mv a0, %[code]": :[code] "r" (0x21));
    __asm__ volatile ("ld a1, 0x00(sp)");
    __asm__ volatile ("ld a2, 0x08(sp)");

    // pop args from stack
    __asm__ volatile ("addi sp, sp, 16");

    __asm__ volatile ("ecall");


    void* retVal;
    __asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    if (retVal != nullptr)
        return 0;
    else{
        return -1;
    }
}

int sem_close (sem_t handle){
    Riscv::forEveryFunc(0x22, handle);
    void *retVal;
    __asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    if (retVal != nullptr)
        return 0;
    else{
        return -1;
    }
}

int sem_wait (sem_t id){
    Riscv::forEveryFunc(0x23, id);
    int  retVal;
    retVal = TCB::running->getRetValForWait();
    //__asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    return retVal;
}

int sem_signal (sem_t id){
    Riscv::forEveryFunc(0x24, id);
    void *retVal;
    __asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    if (retVal != nullptr)
        return 0;
    else{
        return -1;
    }
}

int time_sleep (time_t time){
    Riscv::forEveryFunc(0x31, &time);
    void *retVal;
    __asm__ volatile ("mv %[par], a0": [par]"=r" (retVal));
    if (retVal != nullptr)
        return 0;
    else{
        return -1;
    }
}

//char getc (){
    //Riscv::forEveryFunc(nullptr, 0x41);
//}

void putc(char ch){
    Riscv::forEveryFunc(0x41, &ch);
}

char getc (){
    char *retVal;
    Riscv::forEveryFunc(0x42, nullptr);
    __asm__ volatile ("mv %[par], a0": [par]"=r"(retVal));

    return *retVal;
}

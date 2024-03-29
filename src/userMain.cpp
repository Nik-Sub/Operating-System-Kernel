//
// Created by os on 2/1/23.
//
#include "../h/syscall_c.h"
#include "../h/syscall_cpp.h"
#include "../h/printing.hpp"


//********************************
//TEST 1
/*
struct thread_data {
    int id;
};



class ForkThread : public Thread {
public:
    ForkThread(long _id) noexcept : Thread(), id(_id), finished(false) {}
    virtual void run() {
        printf("Started thread id:", id);

        //if (id == 144)
        //    printf("evo ga", 10);

        ForkThread *thread = new ForkThread(id + 1);

        ForkThread **threads = (ForkThread **) mem_alloc(sizeof(ForkThread *) * id);


        if (threads != nullptr) {
            for (long i = 0; i < id; i++) {
                threads[i] = new ForkThread(id);
            }

            if (thread != nullptr) {
                if (thread->start() == 0) {
                    for (int i = 0; i < 5000; i++) {
                        for (int j = 0; j < 5000; j++) {

                        }
                        thread_dispatch();
                    }

                    while (!thread->isFinished()) {
                        thread_dispatch();
                    }
                }
                delete thread;
            }

            for (long i = 0; i < id; i++) {
                delete threads[i];
            }

            mem_free(threads);
        }
        printf("Finished thread id:", id);

        finished = true;

    }

    bool isFinished() const {
        return finished;
    }

private:
    long id;
    bool finished;
};


void userMain() {
    ForkThread thread(1);

    thread.start();

    while (!thread.isFinished()) {
        thread_dispatch();
    }

    printString("User main finished\n");
}
*/

//********************************
//TEST 2

#include "../h/Slab.h"

#define RUN_NUM (5)
#define ITERATIONS (1000)

#define shared_size (7)
#define MASK (0xA5)

struct data_s {
    int id;
    kmem_cache_t  *shared;
    int iterations;
};

const char * const CACHE_NAMES[] = {"tc_0",
                                    "tc_1",
                                    "tc_2",
                                    "tc_3",
                                    "tc_4"};

void memset(const void *data, int size, int value) {
    for (int j = 0; j < size; j++) {
        ((char *)data)[j]= value;
    }
}

void construct(void *data) {
    static int i = 1;
    printf2("Shared object constructed.\n", i++);
    memset(data, shared_size, MASK);
}

int check(void *data, size_t size) {
    int ret = 1;
    for (size_t i = 0; i < size; i++) {
        if (((unsigned char *)data)[i] != MASK) {
            ret = 0;
        }
    }

    return ret;
}

struct objects_s {
    kmem_cache_t *cache;
    void *data;
};

void work(void* pdata) {
    struct data_s data = *(struct data_s*) pdata;
    int size = 0;
    int object_size = data.id + 1;
    kmem_cache_t *cache = kmem_cache_create(CACHE_NAMES[data.id], object_size, 0, 0);

    struct objects_s *objs = (struct objects_s*)(kmalloc(sizeof(struct objects_s) * data.iterations));

    for (int i = 0; i < data.iterations; i++) {
        if (i % 100 == 0) {
            objs[size].data = kmem_cache_alloc(data.shared);
            objs[size].cache = data.shared;
            if (!check(objs[size].data, shared_size)) {
                printf("Value not correct!", -1);
            }
        }
        else {
            objs[size].data = kmem_cache_alloc(cache);
            objs[size].cache = cache;
            memset(objs[size].data, object_size, MASK);
        }
        size++;
    }

    kmem_cache_info(cache);
    kmem_cache_info(data.shared);

    for (int i = 0; i < size; i++) {
        if (!check(objs[i].data, (cache == objs[i].cache) ? object_size : shared_size)) {
            printf("Value not correct!", -1);
        }
        kmem_cache_free(objs[i].cache, objs[i].data);
    }

    kfree(objs);
    kmem_cache_destroy(cache);
}



void runs(void(*work)(void*), struct data_s* data, int num) {
    for (int i = 0; i < num; i++) {
        struct data_s private_data;
        private_data = *(struct data_s*) data;
        private_data.id = i;
        work(&private_data);
    }
}

void userMain() {
    kmem_cache_t *shared = kmem_cache_create("shared object", shared_size, construct, nullptr);

    struct data_s data;
    data.shared = shared;
    data.iterations = ITERATIONS;
    runs(work, &data, RUN_NUM);

    kmem_cache_destroy(shared);
}

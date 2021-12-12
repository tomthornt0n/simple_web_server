
//~NOTE(tbt): constants

enum{
    WQ_EntryCount = 256,
};

//~NOTE(tbt): data types

typedef void WQ_Work(void *data);

typedef struct{
    WQ_Work *work;
    void *data;
}WQ_Entry;

typedef struct WQ_State WQ_State;

struct WQ_State{
    sem_t semaphore;
    uint32_t volatile next_entry_to_read;
    uint32_t volatile next_entry_to_write;
    uint32_t volatile completion_goal;
    uint32_t volatile completion_count;
    WQ_Entry entries[WQ_EntryCount];
};

//~NOTE(tbt): main API

static void
wq_make(WQ_State *result){
    memset(result, 0, sizeof(*result));
    sem_init(&result->semaphore, 0, 0);
}

// NOTE(tbt): only the main thread should call
static void
wq_push(WQ_State *queue, WQ_Work *work, void *data){
    uint32_t new_next_entry_to_write = (queue->next_entry_to_write + 1) % WQ_EntryCount;
    if(new_next_entry_to_write != queue->next_entry_to_read){
        WQ_Entry *entry = &queue->entries[queue->next_entry_to_write];
        entry->work = work;
        entry->data = data;
        queue->completion_goal += 1;
        __sync_synchronize();
        queue->next_entry_to_write = new_next_entry_to_write;
        sem_post(&queue->semaphore);
    }
}

static bool
wq_do_next_entry(WQ_State *queue){
    bool should_sleep = false;
    uint32_t original_next_entry_to_read = queue->next_entry_to_read;
    uint32_t new_next_entry_to_read = (original_next_entry_to_read + 1) % WQ_EntryCount;
    if(original_next_entry_to_read != queue->next_entry_to_write){
        if(__sync_bool_compare_and_swap(&queue->next_entry_to_read, original_next_entry_to_read, new_next_entry_to_read)){
            WQ_Entry *entry = &queue->entries[original_next_entry_to_read];
            __sync_synchronize();
            entry->work(entry->data);
            __sync_add_and_fetch(&queue->completion_count, 1);
        }
    }else{
        should_sleep = true;
    }
    return should_sleep;
}

static bool
wq_complete_all(WQ_State *queue){
    for(;queue->completion_goal != queue->completion_count;){
        wq_do_next_entry(queue);
    }
    queue->completion_goal = 0;
    queue->completion_count = 0;
}

//~NOTE(tbt): thread pool helpers

static void *
wq_thread_proc(void *arg){
    WQ_State *queue = arg;
    for(;;){
        if(wq_do_next_entry(queue)){
            sem_wait(&queue->semaphore);
        }
    }
    return NULL;
}

static WQ_State *
wq_make_and_spawn_thread_pool(size_t threads_count){
    WQ_State *result = calloc(1, sizeof(*result));
    wq_make(result);
    pthread_t thread_id_buffer;
    for(int thread_index = 0;
        thread_index < threads_count;
        thread_index += 1){
        pthread_create(&thread_id_buffer, NULL, wq_thread_proc, result);
    }
    return result;
}

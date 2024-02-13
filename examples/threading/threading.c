#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg, ...)
// #define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("threading ERROR: " msg "\n", ##__VA_ARGS__)

void *threadfunc(void *thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    pthread_mutex_lock(thread_func_args->mutex);
    usleep(thread_func_args->wait_to_release_ms * 1000);
    pthread_mutex_unlock(thread_func_args->mutex);
    thread_func_args->thread_complete_success = true;
    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if(!data){
        return false;
    }
    
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;
    data->mutex = mutex;

    if(pthread_create(thread, NULL, threadfunc, (void *)data) != 0){
        free(data);
        return false;
    }
    return true;
}
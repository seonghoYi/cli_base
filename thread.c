#include "thread.h"


void* entry(void* args);

int threadCreate(threadHandle_t* p_handle, threadAttr_t attr, void* (* p_thread_main)(void* ), void* p_args)
{
    int ret = 0;
    pthread_attr_t pthread_attr;


    p_handle->p_routine = p_thread_main;
    p_handle->p_args = p_args;
    p_handle->attr = attr;

    pthread_attr_init(&pthread_attr);

    pthread_attr_setscope(&pthread_attr, PTHREAD_SCOPE_PROCESS);

    switch (attr)
    {
    case JOINABLE:
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
        break;
    case DETACHABLE:
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
        break;
    default:
        break;
    }

    ret = pthread_create(&p_handle->id, &pthread_attr, entry, (void* ) p_handle);
    return ret;
}

int threadCancel(threadHandle_t* p_handle)
{
    int ret = 0;

    if (p_handle->state != DONE)
    {
        ret = pthread_cancel(p_handle->id);
    }

    return ret;
}

int threadJoin(threadHandle_t* p_handle)
{
    int ret = 0;
    if (p_handle->attr == JOINABLE && p_handle->state != DONE)
    {
        ret = pthread_join(p_handle->id, NULL);
    }

    return ret;
}


void* entry(void* args)
{
    threadHandle_t* p_handle = (threadHandle_t* )args;
    void* (*p_thread_main)(void* ) = p_handle->p_routine;

    p_handle->state = RUNNING;

    void* ret = p_thread_main(p_handle->p_args);
    
    p_handle->state = DONE;

    return ret;
}
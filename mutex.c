#include "mutex.h"



int mutexCreate(mutex_t* p_mutex)
{
    int ret = 0;

    ret = pthread_mutex_init(p_mutex, NULL);

    return ret;
}

int mutexDestroy(mutex_t* p_mutex)
{
    int ret = 0;

    ret = pthread_mutex_destroy(p_mutex);

    return ret;
}

int mutexLock(mutex_t* p_mutex)
{
    int ret = 0;
    
    ret = pthread_mutex_lock(p_mutex);

    return ret;
}

int mutexUnlock(mutex_t* p_mutex)
{
    int ret = 0;
    
    ret = pthread_mutex_unlock(p_mutex);

    return ret;
}

int mutexTryLock(mutex_t* p_mutex)
{
    int ret = 0;
    
    ret = pthread_mutex_trylock(p_mutex);

    return ret;
}
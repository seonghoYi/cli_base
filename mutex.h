#include <pthread.h>

typedef pthread_mutex_t mutex_t;

int mutexCreate(mutex_t* p_mutex);
int mutexDestroy(mutex_t* p_mutex);

int mutexLock(mutex_t* p_mutex);
int mutexUnlock(mutex_t* p_mutex);
int mutexTryLock(mutex_t* p_mutex);
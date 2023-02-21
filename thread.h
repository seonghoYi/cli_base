#include <pthread.h>


typedef pthread_t threadID_t;

typedef enum _threadState
{
    RUNNING = 0U,
    DONE
} threadState_t;

typedef enum _threadAttr
{
    JOINABLE = 0U,
    DETACHABLE,
} threadAttr_t;




typedef struct _threadHandle
{

    threadID_t id;
    threadAttr_t attr;
    threadState_t state;

    void* p_args;
    void* (* p_routine)(void* );
    
} threadHandle_t;



int threadCreate(threadHandle_t* p_handle, threadAttr_t attr, void* (* p_thread_main)(void* ), void* p_args);
int threadCancel(threadHandle_t* p_handle);

int threadJoin(threadHandle_t* p_handle);

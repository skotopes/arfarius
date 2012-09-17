#include "avmutex.h"

AVMutex::AVMutex(): mutex()
{
    pthread_mutex_init(&mutex, NULL);
}

AVMutex::~AVMutex()
{
    pthread_mutex_destroy(&mutex);
}

bool AVMutex::lock()
{
    if (pthread_mutex_lock(&mutex) != 0) {
        return false;
    }

    return true;
}

bool AVMutex::unlock()
{
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        return false;
    }

    return true;
}

#ifndef AVMUTEX_H
#define AVMUTEX_H

#include <pthread.h>

class AVMutex
{
public:
    explicit AVMutex();
    virtual ~AVMutex();

    bool lock();
    bool unlock();

protected:
    pthread_mutex_t mutex;
};

#endif // AVMUTEX_H

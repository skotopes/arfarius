#include "avmutex.h"
#include <cstdlib>

AVMutex::AVMutex()
    : mutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if(pthread_mutex_init(&mutex, &attr) != 0) {
        std::abort();
    }
    pthread_mutexattr_destroy(&attr);
}

AVMutex::~AVMutex() {
    pthread_mutex_destroy(&mutex);
}

bool AVMutex::lock() {
    if(pthread_mutex_lock(&mutex) != 0) {
        return false;
    }

    return true;
}

bool AVMutex::unlock() {
    if(pthread_mutex_unlock(&mutex) != 0) {
        return false;
    }

    return true;
}

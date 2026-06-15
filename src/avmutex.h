#ifndef AVMUTEX_H
#define AVMUTEX_H

#include <pthread.h>

class AVMutex {
public:
    explicit AVMutex();
    virtual ~AVMutex();

    AVMutex(const AVMutex&) = delete;
    AVMutex& operator=(const AVMutex&) = delete;

    bool lock();
    bool unlock();

protected:
    pthread_mutex_t mutex;
};

class AVLockGuard {
public:
    explicit AVLockGuard(AVMutex& m)
        : _m(m) {
        _m.lock();
    }
    ~AVLockGuard() {
        _m.unlock();
    }
    AVLockGuard(const AVLockGuard&) = delete;
    AVLockGuard& operator=(const AVLockGuard&) = delete;

private:
    AVMutex& _m;
};

#endif // AVMUTEX_H

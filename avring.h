#ifndef AVRING_H
#define AVRING_H

#include "memring.h"
#include "avcondition.h"

class AVRing
{
public:
    AVRing(size_t size);
    virtual ~AVRing();

    // Will slow down performance
    void setPushBarrier();
    void setPullBarrier();

    size_t push(float *buffer, size_t size);
    size_t pull(float *buffer, size_t size);

private:
    MemRing<float> *_ring;

    AVCondition *_barrier_condition;

    bool    _barrier_push_enabled;
    bool    _barrier_pull_enabled;

    size_t _barrier_push_need;
    size_t _barrier_pull_need;
};

#endif // AVRING_H

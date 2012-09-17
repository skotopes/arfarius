#ifndef AVCONDITION_H
#define AVCONDITION_H

#include "avmutex.h"

class AVCondition : public AVMutex
{
public:
    explicit AVCondition();
    virtual ~AVCondition();

    bool wait();
    bool signal();

protected:
    pthread_cond_t condition;
};

#endif // AVCONDITION_H

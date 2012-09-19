#include "avcondition.h"

AVCondition::AVCondition() :
    AVMutex(), condition()
{
    pthread_cond_init(&condition, NULL);
}

AVCondition::~AVCondition()
{
    pthread_cond_destroy(&condition);
}

bool AVCondition::wait()
{
    pthread_cond_wait(&condition, &mutex);
    return true;
}

bool AVCondition::signal()
{
    pthread_cond_signal(&condition);
    return true;
}

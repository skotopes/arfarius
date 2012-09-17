#include "avthread.h"
#include "avexception.h"
#include <iostream>

AVThread::AVThread():
    thread(), attr(), running(false)
{
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}

AVThread::~AVThread()
{
    if (running) {
        std::cerr << "AVThread: destructor called before joining" << std::endl;
    }
}


bool AVThread::create()
{
    if (running)
        return false;

    int ec = pthread_create(&thread, &attr, aCallback, this);
    if (ec != 0)
        return false;

    return true;
}

bool AVThread::join()
{
    int ec = pthread_join(thread, NULL);
    if (ec != 0)
        return false;

    return true;
}

void *AVThread::aCallback(void *data)
{
    AVThread *me = static_cast<AVThread *>(data);

    me->running = true;
    me->run();
    me->running = false;

    pthread_exit(NULL);
}

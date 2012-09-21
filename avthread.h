#ifndef AVTHREAD_H
#define AVTHREAD_H

#include <pthread.h>

class AVThread
{
public:
    explicit AVThread();
    virtual ~AVThread();

    bool create();
    bool join();

    inline bool isRunning() { return running; }

protected:
    virtual void run()=0;

private:
    static void *aCallback(void *data);

    pthread_t thread;
    pthread_attr_t attr;
    volatile bool running;
};

#endif // AVTHREAD_H

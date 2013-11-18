#ifndef AVRING_H
#define AVRING_H

#include "memring.h"
#include "avcondition.h"
#include "avobject.h"

class AVRing: public AVObject
{
public:
    AVRing(size_t size);
    virtual ~AVRing();

    const char * getName() { return "AVRing"; }

    size_t push(float *buffer, size_t size);
    size_t pull(float *buffer, size_t size);

private:
    MemRing<float> *_ring;
};

#endif // AVRING_H

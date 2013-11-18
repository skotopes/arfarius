#include "avring.h"

AVRing::AVRing(size_t size) :
    AVObject(),
    _ring(new MemRing<float>(size))
{

}

AVRing::~AVRing()
{
    delete _ring;
}


size_t AVRing::push(float *buffer, size_t size)
{
    return _ring->push(buffer, size);
}

size_t AVRing::pull(float *buffer, size_t size)
{
    return _ring->pull(buffer, size);
}

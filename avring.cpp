#include "avring.h"

AVRing::AVRing(size_t size) :
    _ring(new MemRing<float>(size)),
    _barrier_condition(0),
    _barrier_push_enabled(false), _barrier_pull_enabled(false),
    _barrier_push_need(0), _barrier_pull_need(0)
{

}

AVRing::~AVRing()
{
    delete _barrier_condition;
    delete _ring;
}

void AVRing::setPushBarrier() {
    if (!_barrier_condition)
        _barrier_condition = new AVCondition();
    _barrier_push_enabled = true;
}

void AVRing::setPullBarrier() {
    if (!_barrier_condition)
        _barrier_condition = new AVCondition();
    _barrier_pull_enabled = true;

}

size_t AVRing::push(float *buffer, size_t size)
{
    if (_barrier_condition) {
        _barrier_condition->lock();
        if (_barrier_push_enabled && _ring->writeSpace() < size) {
            _barrier_push_need = size;
            _barrier_condition->wait();
        }
    }

    size_t ret = _ring->push(buffer, size);

    if (_barrier_pull_need && _ring->readSpace() >= _barrier_pull_need) {
        _barrier_pull_need = 0;
        _barrier_condition->signal();
    }

    if (_barrier_condition)
        _barrier_condition->unlock();

    return ret;
}

size_t AVRing::pull(float *buffer, size_t size)
{
    if (_barrier_condition) {
        _barrier_condition->lock();
        if (_barrier_pull_enabled && _ring->writeSpace() < size) {
            _barrier_push_need = size;
            _barrier_condition->wait();
        }
    }

    size_t ret = _ring->pull(buffer, size);

    if (_barrier_push_need && _ring->writeSpace() >= _barrier_push_need) {
        _barrier_push_need = 0;
        _barrier_condition->signal();
    }

    if (_barrier_condition)
        _barrier_condition->unlock();

    return ret;
}

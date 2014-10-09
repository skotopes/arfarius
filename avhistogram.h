#ifndef AVHISTOGRAM_H
#define AVHISTOGRAM_H

#include "avobject.h"
#include <functional>

class AVHistogram : public AVObject {
public:
    std::function<void(float, float, float, float)> dataCallback;

    AVHistogram(size_t window_size, float threshold=0);
    virtual ~AVHistogram();

    const char * getName() { return "AVHistogram"; }

    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

private:
    size_t _window_size;
    float _threshold;
    size_t _cnt_in, _pos_cnt, _neg_cnt;
    float _pos_peak, _neg_peak, _pos_rms, _neg_rms;

    void _processDb();
    void _processLinear();
};

#endif

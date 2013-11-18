#ifndef AVHISTOGRAM_H
#define AVHISTOGRAM_H

#include "avobject.h"
#include <deque>

class AVHistogram : public AVObject {
public:
    AVHistogram(size_t window_size, float threshold=0);
    virtual ~AVHistogram();

    const char * getName() { return "AVHistogram"; }

    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

    std::deque<float> *getData();

private:
    float _threshold;
    size_t _window_size, _block_current_pos, _block_number, _pos_cnt, _neg_cnt;
    float _pos_peak, _neg_peak, _pos_rms, _neg_rms;
    std::deque<float> _data;

    void _processDb();
    void _processLinear();
};

#endif

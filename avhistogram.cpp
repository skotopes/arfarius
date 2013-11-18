#include "avhistogram.h"
#include "avfile.h"
#include "math.h"

#include <iostream>

AVHistogram::AVHistogram(size_t window_size, float threshold):
    _threshold(threshold), _window_size(window_size),
    _block_current_pos(0), _block_number(0), _pos_cnt(0), _neg_cnt(0),
    _pos_peak(0), _neg_peak(0), _pos_rms(0), _neg_rms(0)
{
}

AVHistogram::~AVHistogram() {
}

size_t AVHistogram::pull(av_sample_t *buffer_ptr, size_t buffer_size)
{
    return 0;
}

size_t AVHistogram::push(float *buffer_ptr, size_t buffer_size) {
    size_t to_consume = buffer_size;
    while (to_consume-- > 0) {
        float v = *buffer_ptr++;

        if (v > 0) {
            if (v > _pos_peak) _pos_peak = v;
            _pos_rms += v*v;
            _pos_cnt++;
        } else {
            if ((-v) > _neg_peak) _neg_peak = -v;
            _neg_rms += v*v;
            _neg_cnt++;
        }

        if (++_block_current_pos == _window_size) {
            if (_threshold != 0.0f) {
                _processDb();
            } else {
                _processLinear();
            }

            _data.push_back(_pos_peak);
            _data.push_back(_neg_peak);
            _data.push_back(_pos_rms);
            _data.push_back(_neg_rms);

            _pos_peak = _neg_peak = 0;
            _pos_rms = _neg_rms = 0;
            _pos_cnt = _neg_cnt = 0;
            _block_current_pos = 0;
            _block_number++;
        }
    }
    
    return buffer_size;
}

std::deque<float> *AVHistogram::getData()
{
    return &_data;
}

void AVHistogram::_processDb() {
    // Peak value
    _pos_peak = 10 * log10f(_pos_peak);
    _neg_peak = 10 * log10f(_neg_peak);

    // RMS value
    _pos_rms = 10 * log10f(sqrtf(_pos_rms / _pos_cnt));
    _neg_rms = 10 * log10f(sqrtf(_neg_rms / _neg_cnt));
}

void AVHistogram::_processLinear() {
    // Block peak value
    _pos_peak = sqrtf(_pos_peak);
    _neg_peak = sqrtf(_neg_peak);

    // Block RMS value
    _pos_rms = _pos_cnt ? sqrtf(sqrtf(_pos_rms/_pos_cnt)) : 0;
    _neg_rms = _neg_cnt ? sqrtf(sqrtf(_neg_rms/_neg_cnt)) : 0;
}


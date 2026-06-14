#include "avhistogram.h"
#include "avfile.h"
#include <cmath>

AVHistogram::AVHistogram(size_t window_size, float threshold)
    : dataCallback(nullptr)
    , _window_size(window_size)
    , _threshold(threshold)
    , _cnt_in(0)
    , _pos_cnt(0)
    , _neg_cnt(0)
    , _pos_peak(0)
    , _neg_peak(0)
    , _pos_rms(0)
    , _neg_rms(0) {
}

AVHistogram::~AVHistogram() {
}

size_t AVHistogram::pull(av_sample_t* /*buffer_ptr*/, size_t /*buffer_size*/) {
    return 0;
}

size_t AVHistogram::push(float* buffer_ptr, size_t buffer_size) {
    if(!buffer_ptr || _window_size == 0) {
        return 0;
    }

    size_t to_consume = buffer_size;
    while(to_consume-- > 0) {
        float v = *buffer_ptr++;

        if(v > 0) {
            if(v > _pos_peak) _pos_peak = v;
            _pos_rms += v * v;
            _pos_cnt++;
        } else {
            if(-v > _neg_peak) _neg_peak = -v;
            _neg_rms += v * v;
            _neg_cnt++;
        }

        if(++_cnt_in == _window_size) {
            if(_threshold != 0.0f) {
                _processDb();
            } else {
                _processLinear();
            }

            if(dataCallback) {
                dataCallback(_pos_peak, _neg_peak, _pos_rms, _neg_rms);
            }

            _pos_peak = _neg_peak = 0;
            _pos_rms = _neg_rms = 0;
            _pos_cnt = _neg_cnt = 0;
            _cnt_in = 0;
        }
    }

    return buffer_size;
}

void AVHistogram::_processDb() {
    const float kMinDb = -120.0f;

    // Peak value (amplitude → dB: 20 * log10)
    if(_pos_peak > 0) {
        _pos_peak = 20.0f * std::log10f(_pos_peak);
    } else {
        _pos_peak = kMinDb;
    }
    if(_neg_peak > 0) {
        _neg_peak = 20.0f * std::log10f(_neg_peak);
    } else {
        _neg_peak = kMinDb;
    }

    // RMS value (amplitude → dB: 20 * log10(rms))
    if(_pos_cnt > 0) {
        _pos_rms = 20.0f * std::log10f(std::sqrtf(_pos_rms / _pos_cnt));
    } else {
        _pos_rms = kMinDb;
    }
    if(_neg_cnt > 0) {
        _neg_rms = 20.0f * std::log10f(std::sqrtf(_neg_rms / _neg_cnt));
    } else {
        _neg_rms = kMinDb;
    }
}

void AVHistogram::_processLinear() {
    // Peak value — raw maximum amplitude
    // _pos_peak and _neg_peak already hold the max absolute value
    if(_pos_cnt == 0) _pos_peak = 0;
    if(_neg_cnt == 0) _neg_peak = 0;

    // RMS value — standard sqrt(mean(v^2))
    if(_pos_cnt > 0) {
        _pos_rms = std::sqrtf(std::sqrtf(_pos_rms / _pos_cnt));
    } else {
        _pos_rms = 0;
    }
    if(_neg_cnt > 0) {
        _neg_rms = std::sqrtf(std::sqrtf(_neg_rms / _neg_cnt));
    } else {
        _neg_rms = 0;
    }
}

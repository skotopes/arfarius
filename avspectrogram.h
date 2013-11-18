#ifndef AVSPECTROGRAM_H
#define AVSPECTROGRAM_H

#include "avobject.h"
#include <deque>

class AVSpectrogram : public AVObject
{
public:
    enum WindowType {
        Square,
        Hann,
        Hamming,
        Blackman,
        BlackmanHarris
    };

    AVSpectrogram(size_t window_size=4096, WindowType window_type=Square, float threshold=0);
    virtual ~AVSpectrogram();

    const char * getName() { return "AVSpectrogram"; }

    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

    std::deque<float> *getData();

private:
    size_t _window_size;
    WindowType _window_type;
    float _threshold;

    float *_in_r, *_in_i, *_out_r, *_out_i;
    size_t _cnt;

    size_t _low_cnt, _mid_cnt, _high_cnt;
    float _low_rms, _mid_rms, _high_rms;
    std::deque<float> _data;

    void _processDomain();

    void _windowHann();
    void _windowHamming();
    void _windowBlackman();
    void _windowBlackmanHarris();

    void _fft(bool inverse=false);
    void _postProcess();

    void _processDb();
    void _processLinear();
};

#endif // AVSPECTROGRAM_H

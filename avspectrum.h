#ifndef AVSPECTRUM_H
#define AVSPECTRUM_H

#include "avobject.h"
#include <functional>
#include <fftw3.h>

class AVSpectrum: public AVObject
{
public:
    enum WindowType {
        Square,
        Hann,
        Hamming,
        Blackman,
        BlackmanHarris
    };

    std::function<void(float, float, float)> dataCallback;

    AVSpectrum(size_t window_size=4096, WindowType window_type=Square, float threshold=0);
    virtual ~AVSpectrum();

    const char * getName() { return "AVSpectrogram"; }

    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

private:
    size_t _window_size;
    WindowType _window_type;
    float _threshold;

    size_t _cnt_in;
    fftwf_plan _plan;

    fftwf_complex *_in;
    fftwf_complex *_out;

    size_t _low_cnt, _mid_cnt, _high_cnt;
    float _low_rms, _mid_rms, _high_rms;

    void _processDomain();

    void _windowHann();
    void _windowHamming();
    void _windowBlackman();
    void _windowBlackmanHarris();

    void _postProcess();
    void _processDb();
    void _processLinear();

};

#endif // AVSPECTRUM_H

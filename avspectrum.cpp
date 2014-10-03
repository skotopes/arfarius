#include "avspectrum.h"
#include "avexception.h"

#include <QDebug>
#include <math.h>

AVSpectrum::AVSpectrum(size_t window_size, WindowType window_type, float threshold):
    _window_size(window_size), _window_type(window_type), _threshold(threshold),
    _cnt_in(0), _plan(nullptr),
    _in(nullptr), _out(nullptr),
    _low_cnt(0), _mid_cnt(0), _high_cnt(0),
    _low_rms(0), _mid_rms(0), _high_rms(0)
{
    fftwf_init_threads();
    fftwf_plan_with_nthreads(4);

    _in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * _window_size);
    _out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * _window_size);
    _plan = fftwf_plan_dft_1d(_window_size, _in, _out, FFTW_FORWARD, FFTW_MEASURE | FFTW_DESTROY_INPUT);
}

AVSpectrum::~AVSpectrum()
{
    if (_plan) fftwf_destroy_plan(_plan);
    if (_out) fftwf_free(_out);
    if (_in) fftwf_free(_in);
    fftwf_cleanup_threads();
}

size_t AVSpectrum::pull(float */*buffer_ptr*/, size_t /*buffer_size*/)
{
    return 0;
}

size_t AVSpectrum::push(float *buffer_ptr, size_t buffer_size)
{
    for (size_t i=0; i<buffer_size; i++) {
        _in[_cnt_in][0] = buffer_ptr[i];
        _in[_cnt_in][1] = 0;

        _cnt_in += 1;
        if (_cnt_in == _window_size) {
            _processDomain();
            _cnt_in = 0;
        }
    }

    return buffer_size;
}

std::deque<float> * AVSpectrum::getData()
{
    return &_data;
}

void AVSpectrum::_processDomain()
{
    switch (_window_type) {
    case Hann:
        _windowHann();
        break;
    case Hamming:
        _windowHamming();
        break;
    case Blackman:
        _windowBlackman();
        break;
    case BlackmanHarris:
        _windowBlackmanHarris();
        break;
    default:
        break;
    }

    fftwf_execute(_plan);

    _postProcess();

    if (_threshold != 0.0f) {
        _processDb();
    } else {
        _processLinear();
    }

    _data.push_back(_low_rms);
    _data.push_back(_mid_rms);
    _data.push_back(_high_rms);

    // nullify variables
    _low_rms = _mid_rms = _high_rms = 0;
    _low_cnt = _mid_cnt = _high_cnt = 0;

}

void AVSpectrum::_windowHann() {
    for (size_t x = 0; x < _window_size; x++) {
        _in[x][0] = _in[x][0] * (0.5 -
                               0.5 * cosf(2 * M_PI * x / (_window_size - 1))
                               );
    }
}

void AVSpectrum::_windowHamming() {
    for (size_t x = 0; x < _window_size; x++) {
        _in[x][0] = _in[x][0] * (0.54 -
                               0.46 * cosf(2 * M_PI * x / (_window_size - 1))
                               );
    }
}

void AVSpectrum::_windowBlackman() {
    for (size_t x = 0; x < _window_size; x++) {
        _in[x][0] = _in[x][0] * (0.42659 -
                               0.49656 * cosf(2 * M_PI * x / (_window_size - 1)) +
                               0.07685 * cosf(4 * M_PI * x / (_window_size - 1))
                               );
    }
}

void AVSpectrum::_windowBlackmanHarris() {
    for (size_t x = 0; x < _window_size; x++) {
        _in[x][0] = _in[x][0] * (0.35875 -
                               0.48829 * cosf(2 * M_PI * x / (_window_size - 1)) +
                               0.14128 * cosf(4 * M_PI * x / (_window_size - 1)) -
                               0.01168 * cosf(6 * M_PI * x / (_window_size - 1))
                               );
    }
}

void AVSpectrum::_postProcess() {
    // amplitude
    for (size_t x = 0; x < _window_size/2; x++) {
        float magnitude = sqrtf(_out[x][0] * _out[x][0] + _out[x][1] * _out[x][1]) * sqrtf(2) / _window_size;
        float frequencie = (float)x * _sample_rate / _window_size;
        if (frequencie > 1500.0f) {
            // high frequency
            _high_rms += magnitude;
            _high_cnt ++;
        } else if (frequencie > 120.0f) {
            // mid frequency
            _mid_rms += magnitude;
            _mid_cnt ++;
        } else {
            // low frequency
            _low_rms += magnitude;
            _low_cnt ++;
        }
    }
}

void AVSpectrum::_processDb() {
    _low_rms = 10 * log10f(_low_rms / _low_cnt);
    _mid_rms = 10 * log10f(_mid_rms / _mid_cnt);
    _high_rms = 10 * log10f(_high_rms / _high_cnt);

    if (_low_rms > _threshold) {
        _low_rms = (_low_rms - _threshold) / - _threshold;
    } else {
        _low_rms = 0;
    }

    if (_mid_rms > _threshold) {
        _mid_rms = (_mid_rms - _threshold) / - _threshold;
    } else {
        _mid_rms = 0;
    }

    if (_high_rms > _threshold) {
        _high_rms = (_high_rms - _threshold) / - _threshold;
    } else {
        _high_rms = 0;
    }
}

void AVSpectrum::_processLinear() {
    _low_rms = _low_rms / _low_cnt;
    _mid_rms = _mid_rms / _mid_cnt;
    _high_rms = _high_rms / _high_cnt;
}

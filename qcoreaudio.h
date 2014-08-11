#ifndef QCOREAUDIO_H
#define QCOREAUDIO_H

#include <QObject>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include "avobject.h"

class QCoreAudio : public QObject, public AVObject
{
    Q_OBJECT
    enum State {
        Stop,
        Play
    };

public:
    explicit QCoreAudio(QObject *parent = 0);
    virtual ~QCoreAudio();

    virtual const char * getName();
    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

    static AudioDeviceID getDefaultOutputDeviceID();

private:
    AudioDeviceID device_id;
    AudioUnit device_unit;
    State state;

    static OSStatus outputCallback(
            void *inRefCon,
            AudioUnitRenderActionFlags *ioActionFlags,
            const AudioTimeStamp *inTimeStamp,
            UInt32 inBusNumber,
            UInt32 inNumberFrames,
            AudioBufferList *ioData);

signals:

public slots:
    bool open(AudioDeviceID dev_id=getDefaultOutputDeviceID());
    void start();
    void stop();
    void close();

    Float64 getDeviceSampleRate();
    void setDeviceSampleRate(Float64 sample_rate);
    UInt32 getDeviceBufferSize();
    void setDeviceBufferSize(UInt32 buffer_size);

};

#endif // QCOREAUDIO_H

#ifndef QCOREAUDIO_H
#define QCOREAUDIO_H

#include <QObject>
#include <CoreAudio/CoreAudio.h>
#include <avobject.h>

class QCoreAudio : public QObject, public AVObject
{
    Q_OBJECT
public:
    explicit QCoreAudio(QObject *parent = 0, AudioDeviceID dev_id=getDefaultOutputDeviceID());
    virtual ~QCoreAudio();

    virtual const char * getName();
    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

    static AudioDeviceID getDefaultOutputDeviceID();

private:
    AudioDeviceIOProcID ioproc_id;
    AudioDeviceID device_id;

    static OSStatus outputCallback(
            AudioObjectID           inDevice,
            const AudioTimeStamp*   inNow,
            const AudioBufferList*  inInputData,
            const AudioTimeStamp*   inInputTime,
            AudioBufferList*        outOutputData,
            const AudioTimeStamp*   inOutputTime,
            void*                   inClientData);

signals:

public slots:
    void start();
    void stop();

    UInt32 getDeviceChannelsCount();
    void setDeviceChannelsCount(UInt32 channels_count);
    Float64 getDeviceSampleRate();
    void setDeviceSampleRate(Float64 sample_rate);
    UInt32 getDeviceBufferSize();
    void setDeviceBufferSize(UInt32 buffer_size);

};

#endif // QCOREAUDIO_H

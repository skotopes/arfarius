#include "qcoreaudio.h"
#include <QDebug>

OSStatus QCoreAudio::outputCallback(
        AudioObjectID           /*inDevice*/,
        const AudioTimeStamp*   /*inNow*/,
        const AudioBufferList*  /*inInputData*/,
        const AudioTimeStamp*   /*inInputTime*/,
        AudioBufferList*        outOutputData,
        const AudioTimeStamp*   /*inOutputTime*/,
        void*                   inClientData)
{
    QCoreAudio *me = reinterpret_cast<QCoreAudio*>(inClientData);

    for (UInt32 i=0; i < outOutputData->mNumberBuffers; i++) {
        AudioBuffer* buf = &outOutputData->mBuffers[i];
        av_sample_t* buffer = reinterpret_cast<av_sample_t*>(buf->mData);

        size_t nBufferFrames = buf->mDataByteSize / sizeof(av_sample_t);
        size_t ret = me->_input->pull(buffer, nBufferFrames);
        if (ret != nBufferFrames) {
            buffer += ret;
            memset(buffer, 0, (nBufferFrames - ret) * sizeof(av_sample_t));
            qWarning() << me << "outputCallback(): Buffer underrun";
        }
    }

    return noErr;
}

QCoreAudio::QCoreAudio(QObject *parent, AudioDeviceID dev_id) :
    QObject(parent), ioproc_id(0), device_id(dev_id)
{
    CFRunLoopRef theRunLoop = NULL;
    AudioObjectPropertyAddress property = { kAudioHardwarePropertyRunLoop,
                                            kAudioObjectPropertyScopeGlobal,
                                            kAudioObjectPropertyElementMaster };
    OSStatus ret = AudioObjectSetPropertyData( kAudioObjectSystemObject, &property, 0, NULL, sizeof(CFRunLoopRef), &theRunLoop);
    if (ret != noErr) {
        qWarning() << this << "QCoreAudio(): AudioObjectSetPropertyData error" << ret;
    }

    device_id = dev_id;
    ret = AudioDeviceCreateIOProcID(
                device_id,
                outputCallback,
                reinterpret_cast<void*>(this),
                &ioproc_id
                );

    if (ret != noErr) {
        qWarning() << this << "QCoreAudio(): AudioDeviceCreateIOProcID error" << ret;
    }
}

QCoreAudio::~QCoreAudio()
{
    OSStatus ret = AudioDeviceDestroyIOProcID(device_id, ioproc_id);
    if (ret != noErr) {
        qWarning() << this << "close(): AudioDeviceDestroyIOProcID error" << ret;
    }
}

const char * QCoreAudio::getName()
{
    return "QCoreAudio";
}

size_t QCoreAudio::pull(float */*buffer_ptr*/, size_t /*buffer_size*/)
{
    return 0;
}

size_t QCoreAudio::push(float */*buffer_ptr*/, size_t /*buffer_size*/)
{
    return 0;
}

AudioDeviceID QCoreAudio::getDefaultOutputDeviceID()
{
    AudioDeviceID dev_id = 0;
    UInt32 dev_id_size = sizeof(AudioDeviceID);

    AudioObjectPropertyAddress theAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    OSStatus ret = AudioObjectGetPropertyData(
                kAudioObjectSystemObject,
                &theAddress,
                0,
                NULL,
                &dev_id_size,
                &dev_id
                );

    if (ret != noErr) {
        qWarning() << "getDefaultOutputDeviceID(): AudioObjectGetPropertyData error " << ret;
    }

    return dev_id;
}

void QCoreAudio::start()
{
    OSStatus ret = AudioDeviceStart(device_id, ioproc_id);
    if (ret != noErr) {
        qWarning() << this << "open(): AudioDeviceStart error" << ret;
    }
}

void QCoreAudio::stop()
{
    OSStatus ret = AudioDeviceStop(device_id, ioproc_id);
    if (ret != noErr) {
        qWarning() << this << "close(): AudioDeviceStop error" << ret;
    }
}

UInt32 QCoreAudio::getDeviceChannelsCount()
{
    UInt32 buffers_size = 0;
    AudioObjectPropertyAddress property_address = {
        kAudioDevicePropertyStreamConfiguration,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    OSStatus ret = AudioObjectGetPropertyDataSize(device_id, &property_address, 0, NULL, &buffers_size);
    if (ret != noErr) {
        qWarning() << "getDeviceChannelsCount(): AudioObjectGetPropertyDataSize error " << ret;
    }

    AudioBufferList	*buffers = (AudioBufferList *) malloc(buffers_size);
    ret = AudioObjectGetPropertyData(device_id, &property_address, 0, NULL, &buffers_size, buffers);
    if (ret != noErr) {
        qWarning() << "getDeviceChannelsCount(): AudioObjectGetPropertyData error " << ret;
    }
    UInt32  mNumberChannels = buffers->mBuffers[0].mNumberChannels;
    free(buffers);

    return mNumberChannels;
}

void QCoreAudio::setDeviceChannelsCount(UInt32 channels_count)
{
    AudioStreamBasicDescription	description;
    UInt32 description_size = sizeof(AudioStreamBasicDescription);

    AudioObjectPropertyAddress property_address = {
        kAudioStreamPropertyVirtualFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMaster
    };

    OSStatus ret = AudioObjectGetPropertyData(device_id, &property_address, 0, NULL, &description_size, &description);
    if (ret != noErr) {
        qWarning() << "setDeviceChannelsCount(): AudioObjectGetPropertyData error " << ret;
    }

    description.mChannelsPerFrame = channels_count;
    description.mBytesPerFrame = 4 * channels_count;
    description.mBytesPerPacket = description.mBytesPerFrame * description.mFramesPerPacket;
    description.mBitsPerChannel = 32;
    description.mFormatID = kAudioFormatLinearPCM;
    description.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsFloat;

    ret = AudioObjectSetPropertyData(device_id, &property_address, 0, NULL, description_size, &description);
    if (ret != noErr) {
        qWarning() << "setDeviceChannelsCount(): AudioObjectSetPropertyData error " <<  ret;
    }
}

UInt32 QCoreAudio::getDeviceBufferSize() {
    UInt32 buffer_size = 0;
    UInt32 buffer_size_size = sizeof(UInt32);

    AudioObjectPropertyAddress property_address = {
        kAudioDevicePropertyBufferFrameSize,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    OSStatus ret = AudioObjectGetPropertyData(
                device_id,
                &property_address,
                0,
                NULL,
                &buffer_size_size,
                &buffer_size
                );

    if (ret != noErr) {
        qWarning() << "getBufferSize(): AudioObjectGetPropertyData error " << ret;
    }

    return buffer_size;
}

void QCoreAudio::setDeviceBufferSize(UInt32 buffer_size) {
    AudioObjectPropertyAddress property_address = {
        kAudioDevicePropertyBufferFrameSize,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    OSStatus ret = AudioObjectSetPropertyData(device_id, &property_address, 0, NULL, sizeof(buffer_size), &buffer_size);
    if (ret != noErr) {
        qWarning() << this << "setBufferSize(): AudioObjectSetPropertyData error" << ret;
    }
}

Float64 QCoreAudio::getDeviceSampleRate()
{
    Float64 sample_rate = 0;
    UInt32 sample_rate_size = sizeof(Float64);

    AudioObjectPropertyAddress property_address = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    OSStatus ret = AudioObjectGetPropertyData(device_id, &property_address, 0, NULL, &sample_rate_size, &sample_rate);
    if (ret != noErr) {
        qWarning() << "getDeviceSampleRate(): AudioObjectGetPropertyData error " << ret;
    }

    return sample_rate;
}

void QCoreAudio::setDeviceSampleRate(Float64 sample_rate)
{
    AudioObjectPropertyAddress property_address = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    OSStatus ret = AudioObjectSetPropertyData(device_id, &property_address, 0, NULL, sizeof(Float64), &sample_rate);
    if (ret != noErr) {
        qWarning() << "setDeviceSampleRate(): AudioObjectGetPropertyData error " << ret;
    }
}

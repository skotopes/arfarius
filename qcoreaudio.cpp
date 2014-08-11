#include "qcoreaudio.h"
#include <QDebug>

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>

OSStatus QCoreAudio::outputCallback(
        void *inRefCon,
        AudioUnitRenderActionFlags */*ioActionFlags*/,
        const AudioTimeStamp */*inTimeStamp*/,
        UInt32 /*inBusNumber*/,
        UInt32 /*inNumberFrames*/,
        AudioBufferList *ioData
        )
{
    QCoreAudio *me = reinterpret_cast<QCoreAudio*>(inRefCon);

    for (UInt32 i=0; i < ioData->mNumberBuffers; i++) {
        AudioBuffer* buf = &ioData->mBuffers[i];
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

QCoreAudio::QCoreAudio(QObject *parent) :
    QObject(parent), device_id(), state(Stop)
{
    open();
}

QCoreAudio::~QCoreAudio()
{
    close();
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

bool QCoreAudio::open(AudioDeviceID dev_id)
{
    device_id = dev_id;

    ComponentDescription componentDescription;
    componentDescription.componentType = kAudioUnitType_Output;
    componentDescription.componentSubType = kAudioUnitSubType_HALOutput;
    componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    componentDescription.componentFlags = 0;
    componentDescription.componentFlagsMask = 0;

    Component component = FindNextComponent(NULL, &componentDescription);
    if (component == 0) {
        qWarning() << this << "open(): failed to find HAL Output component";
        return false;
    }

    if (OpenAComponent(component, &device_unit) != noErr) {
        qWarning() << this << "open(): unable to Open Output Component";
        return false;
    }


    AURenderCallbackStruct callback;
    callback.inputProc = outputCallback;
    callback.inputProcRefCon = this;
    if (AudioUnitSetProperty(device_unit,
                             kAudioUnitProperty_SetRenderCallback,
                             kAudioUnitScope_Global,
                             0,
                             &callback,
                             sizeof(callback)) != noErr) {
        qWarning() << this << "open(): unable to set callback";
        return false;
    }

    if (AudioUnitSetProperty(device_unit,
                             kAudioOutputUnitProperty_CurrentDevice,
                             kAudioUnitScope_Global,
                             0,
                             &device_id,
                             sizeof(device_id)) != noErr) {
        qWarning() << this << "open(): unable to set device";
        return false;
    }

    AudioStreamBasicDescription m_streamFormat;

    m_streamFormat.mFormatFlags         = kAudioFormatFlagIsPacked;
    m_streamFormat.mSampleRate          = getDeviceSampleRate();
    m_streamFormat.mFramesPerPacket     = 1;
    m_streamFormat.mChannelsPerFrame    = 2;
    m_streamFormat.mBitsPerChannel      = 32;
    m_streamFormat.mBytesPerFrame       = m_streamFormat.mChannelsPerFrame * (m_streamFormat.mBitsPerChannel / 8);
    m_streamFormat.mBytesPerPacket      = m_streamFormat.mFramesPerPacket * m_streamFormat.mBytesPerFrame;
    m_streamFormat.mFormatID            = kAudioFormatLinearPCM;
    m_streamFormat.mFormatFlags         = kAudioFormatFlagIsFloat;

    UInt32 size = sizeof(m_streamFormat);
    if (AudioUnitSetProperty(device_unit,
                                kAudioUnitProperty_StreamFormat,
                                kAudioUnitScope_Input,
                                0,
                                &m_streamFormat,
                                size) != noErr) {
        qWarning() << this << "open(): Unable to Set Stream information";
        return false;
    }

    if (AudioUnitInitialize(device_unit)) {
        qWarning() << this << "open(): Failed to initialize AudioUnit";
        return false;
    }

    return true;
}

void QCoreAudio::start()
{
    if (state != Stop)
        qFatal("Programming error: device is not stopped");

    AudioOutputUnitStart(device_unit);
}

void QCoreAudio::stop()
{
    if (state != Play)
        qFatal("Programming error: device is not playing");

    AudioOutputUnitStop(device_unit);
}

void QCoreAudio::close()
{
    if (state != Stop) stop();
    AudioUnitUninitialize(device_unit);
    CloseComponent(device_unit);
}

UInt32 QCoreAudio::getDeviceBufferSize() {
    UInt32 buffer_size = 0;
    UInt32 size = sizeof(UInt32);
    if (AudioUnitGetProperty(device_unit,
                             kAudioDevicePropertyBufferFrameSize,
                             kAudioUnitScope_Global,
                             0,
                             &buffer_size,
                             &size) != noErr) {
        qWarning() << this << "getDeviceBufferSize(): failed to get buffer size";
        return 0;
    }

    return buffer_size;
}

void QCoreAudio::setDeviceBufferSize(UInt32 buffer_size) {
    UInt32 size = sizeof(UInt32);
    if (AudioUnitSetProperty(device_unit,
                             kAudioDevicePropertyBufferFrameSize,
                             kAudioUnitScope_Global,
                             0,
                             &buffer_size,
                             size) != noErr) {
        qWarning() << this << "setDeviceBufferSize(): failed to set buffer size";
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

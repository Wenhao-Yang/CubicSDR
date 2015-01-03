#pragma once

#include <queue>
#include <vector>
#include <map>
#include <string>
#include <atomic>
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/thread.h"

#include "AudioThread.h"
#include "ThreadQueue.h"
#include "RtAudio.h"
#include "DemodDefs.h"

class AudioThreadInput: public ReferenceCounter {
public:
    int frequency;
    int sampleRate;
    int channels;
    std::vector<float> data;

    AudioThreadInput() :
            frequency(0), sampleRate(0), channels(0) {

    }

    ~AudioThreadInput() {
        std::lock_guard < std::mutex > lock(m_mutex);
    }
};

class AudioThreadCommand {
public:
    enum AudioThreadCommandEnum {
        AUDIO_THREAD_CMD_NULL, AUDIO_THREAD_CMD_SET_DEVICE
    };

    AudioThreadCommand() :
            cmd(AUDIO_THREAD_CMD_NULL), int_value(0) {
    }

    AudioThreadCommandEnum cmd;
    int int_value;
};

typedef ThreadQueue<AudioThreadInput *> AudioThreadInputQueue;
typedef ThreadQueue<AudioThreadCommand> AudioThreadCommandQueue;

class AudioThread {
public:

    AudioThreadInput *currentInput;
    AudioThreadInputQueue *inputQueue;
    std::atomic<unsigned int> audioQueuePtr;
    std::atomic<unsigned int> underflowCount;
    std::atomic<bool> terminated;
    std::atomic<bool> active;
    std::atomic<int> outputDevice;
    float gain;

    AudioThread(AudioThreadInputQueue *inputQueue, DemodulatorThreadCommandQueue* threadQueueNotify);
    ~AudioThread();

    static void enumerateDevices(std::vector<RtAudio::DeviceInfo> &devs);

    void setupDevice(int deviceId);
    void setInitOutputDevice(int deviceId);
    int getOutputDevice();
    void threadMain();
    void terminate();

    bool isActive();
    void setActive(bool state);

    AudioThreadCommandQueue *getCommandQueue();

private:
    RtAudio dac;
    RtAudio::StreamParameters parameters;
    AudioThreadCommandQueue cmdQueue;
    DemodulatorThreadCommandQueue* threadQueueNotify;

#ifdef __APPLE__
public:
    void bindThread(AudioThread *other);
    void removeThread(AudioThread *other);

    static std::map<int,AudioThread *> deviceController;
    static std::map<int,std::thread *> deviceThread;
    static void deviceCleanup();
    std::atomic<std::vector<AudioThread *> *> boundThreads;
#endif
};


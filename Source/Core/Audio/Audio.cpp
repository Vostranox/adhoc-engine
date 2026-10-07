#include "Audio.hpp"

#include "LoadWav.hpp"

#include <Vulkan/Context.hpp>

#include <Event/Event.hpp>

#include <string>

namespace adh {
    AudioDevice::AudioDevice() : device{}, context{} {}

    AudioDevice::AudioDevice(AudioDevice&& rhs) noexcept {
        device      = rhs.device;
        context     = rhs.context;
        rhs.device  = nullptr;
        rhs.context = nullptr;
    }

    AudioDevice& AudioDevice::operator=(AudioDevice&& rhs) noexcept {
        Destroy();
        device      = rhs.device;
        context     = rhs.context;
        rhs.device  = nullptr;
        rhs.context = nullptr;
        return *this;
    }

    AudioDevice::~AudioDevice() { Destroy(); }

    void AudioDevice::Create() ADH_NOEXCEPT {
        device = alcOpenDevice(nullptr);
        ADH_THROW(device, "Failed to open alc device!");
        context = alcCreateContext(device, nullptr);
        ADH_THROW(context, "Failed to create alc context!");
        alcMakeContextCurrent(context);
    }

    void AudioDevice::Destroy() noexcept {
        if (context) {
            alcDestroyContext(context);
        }
        if (device) {
            alcCloseDevice(device);
        }
    }
} // namespace adh

namespace adh {
    Audio::~Audio() {
        alDeleteSources(1, &mSource);
        alDeleteBuffers(1, &mBuffer);
    }

    void Audio::Create(const char* filePath) {
        // Generate buffer
        alGenBuffers(1, &mBuffer);
        std::uint8_t channel, bps;
        std::int32_t sampleRate;
        std::int32_t size;
        // int channel, sampleRate, bps, size;
        char* data = load_wav(filePath, channel, sampleRate, bps, size);
        if (!data) {
            std::string file = filePath;
            auto pos         = file.find("Audio");
            file             = "[" + file.substr(pos + 6) + "] " + "Invalid audio file.";
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, file.data());
            return;
        }

        if (channel == 1) {
            if (bps == 8) {
                mFormat = AL_FORMAT_MONO8;
            } else {
                mFormat = AL_FORMAT_MONO16;
            }
        } else {
            if (bps == 8) {
                mFormat = AL_FORMAT_STEREO8;
            } else {
                mFormat = AL_FORMAT_STEREO16;
            }
        }
        alBufferData(mBuffer, mFormat, data, size, sampleRate);

        // Generate source
        alGenSources(1, &mSource);
        alSourcef(mSource, AL_PITCH, mPitch);
        alSourcef(mSource, AL_GAIN, mGain);
        alSource3f(mSource, AL_POSITION, mPosition[0], mPosition[1], mPosition[2]);
        alSource3f(mSource, AL_VELOCITY, mVelocity[0], mVelocity[1], mVelocity[2]);
        alSourcei(mSource, AL_LOOPING, mLoop);
        alSourcei(mSource, AL_BUFFER, mBuffer);

        delete[] data;

        mReady = true;
    }

    void Audio::Create2(const char* fileName) {
        Create((vk::Context::Get()->GetDataDirectory() + "Assets/Audio/" + fileName).data());
    }

    // void Audio::OnUpdate() {
    //     if (mDuration && IsPlaying()) {
    //         if (std::chrono::duration<float>{ std::chrono::steady_clock::now() -mStart }.count() > mDuration + mPauseDuration.count()) {
    //             Stop();
    //         }
    //     }
    // }

    void Audio::Play() {
        if (mReady) {
            alSourcePlay(mSource);
        }
    }

    void Audio::Stop() {
        if (mReady) {
            alSourceStop(mSource);
        }
    }

    void Audio::Pause() {
        if (mReady) {
            alSourcePause(mSource);
        }
    }

    void Audio::Loop(bool loop) {
        if (mReady) {
            mLoop = loop;
            alSourcei(mSource, AL_LOOPING, mLoop);
        }
    }

    bool Audio::IsPlaying() const noexcept {
        if (mReady) {
            ALint state;
            alGetSourcei(mSource, AL_SOURCE_STATE, &state);

            if (state == AL_PLAYING) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

} // namespace adh

// #include "Audio.hpp"
// #include <Vulkan/Context.hpp>
// #include <iostream>

// #if defined(ADH_APPLE)
// namespace adh {
//     Audio::Audio(const char* filePath) {
//         Create(filePath);
//     }

//     Audio::~Audio() {
//         [mSound stop];
//     }

//     void Audio::Create(const char* filePath, float seconds) {
//         @autoreleasepool {
//             NSString* s = [NSString stringWithUTF8String:filePath];
//             mSound      = [[NSSound alloc] initWithContentsOfFile:s
//                                                  byReference:NO];
//         }

//         mDuration = seconds;
//     }

//     void Audio::Create2(const char* fileName, float seconds) {
//         Create((vk::Context::Get()->GetDataDirectory() + "Assets/Audio/" + fileName).data(), seconds);
//     }

//     void Audio::OnUpdate() {
//         if (mDuration && IsPlaying()) {
//             if (std::chrono::duration<float>{ std::chrono::steady_clock::now() - mStart }.count() > mDuration + mPauseDuration.count()) {
//                 Stop();
//             }
//         }
//         if (mLoop) {
//             if (!IsPlaying()) {
//                 Play();
//             }
//         }
//     }

//     void Audio::Play() {
//         if (!IsPlaying()) {
//             [mSound play];
//         } else {
//             [mSound stop];
//             [mSound play];
//         }
//         mStart  = std::chrono::steady_clock::now();
//         mStatus = Status::ePlaying;
//     }

//     void Audio::Stop() {
//         [mSound stop];
//         mStatus = Status::eStopped;
//     }

//     void Audio::Pause() {
//         [mSound pause];
//         mPause  = std::chrono::steady_clock::now();
//         mStatus = Status::ePaused;
//     }

//     void Audio::Resume() {
//         [mSound resume];
//         mPauseDuration = { std::chrono::steady_clock::now() - mPause };
//         mStatus        = Status::ePlaying;
//     }

//     void Audio::Loop(bool loop) {
//         mLoop = loop;
//     }

//     bool Audio::IsPlaying() const noexcept {
//         return mSound.playing && mStatus == Status::ePlaying;
//     }
// } // namespace adh

// #elif defined(ADH_WINDOWS)
// #    pragma comment(lib, "winmm.lib")

// namespace adh {
//     Audio::Audio(const char* filePath) {
//         Create(filePath);
//     }

//     Audio::~Audio() {
//         mciSendCommand(mWaveOpen.wDeviceID, MCI_CLOSE, NULL, NULL);
//     }

//     void Audio::Create(const char* filePath, float seconds) {
//         std::int32_t size{ MultiByteToWideChar(CP_UTF8, 0, filePath, -1, nullptr, 0) };
//         auto file = new wchar_t[size];
//         MultiByteToWideChar(CP_UTF8, 0, filePath, -1, file, size);

//         mWaveOpen  = MCI_WAVE_OPEN_PARMS{ 0, 0, reinterpret_cast<LPCWSTR>(MCI_DEVTYPE_WAVEFORM_AUDIO), file, 0, 0 };
//         mMciStatus = MCI_STATUS_PARMS{ 0, 0, MCI_STATUS_MODE, 0 };
//         mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE_ID, reinterpret_cast<DWORD_PTR>(&mWaveOpen));

//         delete[] file;

//         mDuration = seconds;
//     }

//     void Audio::Create2(const char* fileName, float seconds) {
//         Create((vk::Context::Get()->GetDataDirectory() + "Assets/Audio/" + fileName).data(), seconds);
//     }

//     void Audio::OnUpdate() {
//         if (mDuration && IsPlaying()) {
//             if (std::chrono::duration<float>{ std::chrono::steady_clock::now() - mStart }.count() > mDuration + mPauseDuration.count()) {
//                 Stop();
//             }
//         }
//         if (mLoop) {
//             if (!IsPlaying()) {
//                 Play();
//             }
//         }
//     }

//     void Audio::Play() {
//         if (!IsPlaying()) {
//             mciSendCommand(mWaveOpen.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
//             mciSendCommand(mWaveOpen.wDeviceID, MCI_PLAY, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE_ID, NULL);
//         } else {
//             mciSendCommand(mWaveOpen.wDeviceID, MCI_STOP, MCI_WAIT, NULL);
//             mciSendCommand(mWaveOpen.wDeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
//             mciSendCommand(mWaveOpen.wDeviceID, MCI_PLAY, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE_ID, NULL);
//         }
//         mStart  = std::chrono::steady_clock::now();
//         mStatus = Status::ePlaying;
//     }

//     void Audio::Stop() {
//         mciSendCommand(mWaveOpen.wDeviceID, MCI_STOP, MCI_WAIT, NULL);
//         mStatus = Status::eStopped;
//     }

//     void Audio::Pause() {
//         mciSendCommand(mWaveOpen.wDeviceID, MCI_PAUSE, MCI_WAIT, NULL);
//         mPause  = std::chrono::steady_clock::now();
//         mStatus = Status::ePaused;
//     }

//     void Audio::Resume() {
//         mciSendCommand(mWaveOpen.wDeviceID, MCI_RESUME, MCI_WAIT, NULL);
//         mPauseDuration = { std::chrono::steady_clock::now() - mPause };
//         mStatus        = Status::ePlaying;
//     }

//     void Audio::Loop(bool loop) {
//         mLoop = loop;
//     }

//     bool Audio::IsPlaying() const noexcept {
//         mciSendCommand(mWaveOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, reinterpret_cast<DWORD_PTR>(&mMciStatus));
//         if (mMciStatus.dwReturn == MCI_MODE_PLAY && mStatus == Status::ePlaying) {
//             return true;
//         } else {
//             return false;
//         }
//     }
// } // namespace adh
// #elif defined(ADH_LINUX)
// namespace adh {
//     Audio::Audio(const char* filePath) {
//     }

//     Audio::~Audio() {
//     }

//     void Audio::Create(const char* filePath, float seconds) {
//     }

//     void Audio::Create2(const char* fileName, float seconds) {
//     }

//     void Audio::OnUpdate() {
//     }

//     void Audio::Play() {
//     }

//     void Audio::Stop() {
//     }

//     void Audio::Pause() {
//     }

//     void Audio::Resume() {
//     }

//     void Audio::Loop(bool loop) {
//     }

//     bool Audio::IsPlaying() const noexcept {
//         return false;
//     }
// } // namespace adh
// #endif

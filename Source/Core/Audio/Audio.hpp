#pragma once
#include <iostream>

#include <atomic>
#include <future>

#include <Utility.hpp>

#include <openal-soft/include/AL/al.h>
#include <openal-soft/include/AL/alc.h>

namespace adh {
    class AudioDevice {
      public:
        AudioDevice();

        AudioDevice(const AudioDevice&)            = delete;
        AudioDevice& operator=(const AudioDevice&) = delete;

        AudioDevice(AudioDevice&& rhs) noexcept;

        AudioDevice& operator=(AudioDevice&&) noexcept;

        ~AudioDevice();

        void Create() ADH_NOEXCEPT;

        void Destroy() noexcept;

      private:
        ALCdevice* device;
        ALCcontext* context;
    };
} // namespace adh

namespace adh {
    class Audio {
      public:
        Audio() = default;

        ~Audio();

        void Create(const char* filePath);

        void Create2(const char* fileName);

        // void OnUpdate();

        void Play();

        void Stop();

        void Pause();

        void Loop(bool loop);

        bool IsPlaying() const noexcept;

      private:
        ALuint mFormat;
        ALuint mSource;
        float mPitch{ 1 };
        float mGain{ 1 };
        float mPosition[3]{};
        float mVelocity[3]{};
        ALuint mBuffer;
        bool mLoop{ false };
        bool mReady{ false };
    };
} // namespace adh

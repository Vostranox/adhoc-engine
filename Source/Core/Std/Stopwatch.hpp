#pragma once
#include <chrono>
#include <thread>

namespace adh {
    template <typename T = float>
    class Stopwatch {
      public:
        using Type     = T;
        using Clock    = std::chrono::steady_clock;
        using Duration = std::chrono::duration<Type>;

      public:
        Stopwatch() noexcept : m_Start{ Clock::now() },
                               m_End{ Clock::now() } {
        }

        auto GetTime() const noexcept {
            return Duration{ Clock::now() - m_Start }.count();
        }

        auto Lap() noexcept {
            const Type lap{ Duration{ Clock::now() - m_End }.count() };
            m_End = Clock::now();
            return lap;
        }

        auto Reset() noexcept {
            const Clock::time_point temp{ m_Start };
            m_Start = m_End = Clock::now();
            return Duration{ m_Start - temp }.count();
        }

        void SleepFor(std::uint32_t milliseconds) const noexcept {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        void SleepUntil(std::uint32_t milliseconds) const noexcept {
            std::this_thread::sleep_until(m_End + std::chrono::milliseconds(milliseconds));
        }

      private:
        Clock::time_point m_Start;
        Clock::time_point m_End;
    };
} // namespace adh

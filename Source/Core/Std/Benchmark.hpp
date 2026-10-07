#pragma once
#include "Stopwatch.hpp"
#include "Utility.hpp"

#include <iostream>

#if !defined(BENCHMARK_BEGIN)
#    define BENCHMARK_BEGIN \
        {                   \
            adh::Stopwatch timer;
#endif

#if !defined(BENCHMARK_END)
#    define BENCHMARK_END                                                            \
        auto result{ timer.GetTime() * 1'000.0f };                                   \
        std::cout << "Benchmark time: [" << result << "] millisecond." << std::endl; \
        }
#endif

namespace adh {
    template <typename Ptr, typename... Args>
    inline void Benchmark(Ptr funcPtr, Args&&... args) {
        Stopwatch timer;
        funcPtr(Forward<Args>(args)...);
        auto result{ timer.GetTime() * 1'000.0f };
        std::cout << "Benchmark time: [" << result << "] millisecond." << std::endl;
    };
} // namespace adh

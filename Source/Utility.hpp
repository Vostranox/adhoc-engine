#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(ADH_THROW)
#    undef ADH_THROW
#endif
#if defined(ADH_DEBUG)
#    define ADH_THROW(expression, message)                     \
        {                                                      \
            if (!(expression)) {                               \
                std::ostringstream str;                        \
                str << "Message:    " << message << "\n\n"     \
                    << "Function:   [ " << __func__ << " ]\n"  \
                    << "Line:       [ " << __LINE__ << " ]\n"  \
                    << "File:       [ " << __FILE__ << " ]\n"; \
                if (!adh::ShowThrowDialog(str.str())) {        \
                    throw std::runtime_error(message);         \
                }                                              \
            }                                                  \
        }
namespace adh {
    bool ShowThrowDialog(const std::string& text) noexcept;
} // namespace adh
#else
#    define ADH_THROW(expression, message) (void)(expression)
#endif // ADH_DEBUG

#if defined(ADH_NOEXCEPT)
#    undef ADH_NOEXCEPT
#endif
#if defined(ADH_DEBUG)
#    define ADH_NOEXCEPT
#else
#    define ADH_NOEXCEPT noexcept
#endif // ADH_DEBUG

#if defined(ADH_OFFSET)
#    undef ADH_OFFSET
#endif
#define ADH_OFFSET(x, y) \
    std::size_t { reinterpret_cast<std::size_t>(&((static_cast<x*>(nullptr)->y))) }

#if defined(ADH_TO_STRING)
#    undef ADH_TO_STRING
#endif
#define ADH_TO_STRING(x) #x

#if defined(ADH_LOG)
#    undef ADH_LOG
#endif
#define ADH_LOG(x) std::cout << x << std::endl

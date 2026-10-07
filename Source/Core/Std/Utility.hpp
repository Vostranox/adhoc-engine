#pragma once
#include <iostream>

namespace adh {
    namespace _internal {
        template <typename T>
        struct RemoveReference {
            using Type = T;
        };

        template <typename T>
        struct RemoveReference<T&> {
            using Type = T;
        };

        template <typename T>
        struct RemoveReference<T&&> {
            using Type = T;
        };

        template <typename T>
        struct PointerDefaultDeleter {
            void operator()(T* ptr) noexcept {
                delete ptr;
            }
        };

        template <typename T>
        struct ArrayDefaultDeleter {
            void operator()(T* ptr) noexcept {
                operator delete(ptr);
            }
        };

        template <typename T>
        class Bucket {
          public:
            using Type      = T;
            using BucketPtr = Bucket*;
            using SizeType  = std::size_t;

          public:
            static constexpr SizeType bucketSize{ sizeof(T) > 8u ? 5u : sizeof(T) > 2u ? 10u
                                                                                       : 20u };

          public:
            Type data[bucketSize];
            BucketPtr next;
            SizeType capacity;
        };
    } // namespace _internal

    template <typename T>
    using RemoveReference = typename _internal::RemoveReference<T>::Type;

    template <typename T>
    using ToPureType = std::remove_pointer_t<std::add_pointer_t<std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>>>;

    template <typename T>
    [[nodiscard]] constexpr decltype(auto) Move(T&& data) noexcept {
        return static_cast<RemoveReference<T>&&>(data);
    }

    template <typename T>
    [[nodiscard]] constexpr decltype(auto) Forward(RemoveReference<T>& data) noexcept {
        return static_cast<T&&>(data);
    }

    template <typename T>
    [[nodiscard]] constexpr decltype(auto) Forward(RemoveReference<T>&& data) noexcept {
        return static_cast<T&&>(data);
    }

    template <typename T>
    constexpr void Swap(T& lhs, T& rhs) noexcept {
        auto temp{ Move(lhs) };
        lhs = Move(rhs);
        rhs = Move(temp);
    }

    template <typename T>
    struct Lesser {
        using Type = T;
        constexpr bool operator()(Type x, Type y) const noexcept {
            return x < y;
        }
    };

    template <typename T>
    struct Greater {
        using Type = T;
        constexpr bool operator()(Type x, Type y) const noexcept {
            return x > y;
        }
    };

    template <typename R, typename... Args>
    void* VoidCast(R (*f)(Args... args)) {
        union {
            R(*pf)
            (Args...);
            void* p;
        };
        pf = f;
        return p;
    }

    template <typename T, typename R, typename... Args>
    void* VoidCast(R (T::*f)(Args... args)) {
        union {
            R(T::*pf)
            (Args...);
            void* p;
        };
        pf = f;
        return p;
    }

    template <typename T, typename R, typename... Args>
    void* VoidCast(R (T::*f)(Args... args) const) {
        union {
            R(T::*pf)
            (Args...) const;
            void* p;
        };
        pf = f;
        return p;
    }
} // namespace adh

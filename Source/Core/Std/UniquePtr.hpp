#pragma once
#include "Concepts.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>

namespace adh {
    template <typename T, typename T2>
    class SharedPtr;

    template <typename T, typename Deleter = _internal::PointerDefaultDeleter<T>>
    class UniquePtr : Deleter {
      public:
        using Type       = T;
        using Pointer    = Type*;
        using CPointer   = const Type*;
        using Reference  = Type&;
        using CReference = const Type&;

        inline static constexpr std::size_t ZeroCount{ 0u };

      public:
        template <typename T3, typename T4>
        friend class SharedPtr;

        template <typename T3, typename T4>
        friend class UniquePtr;

      private:
        using Deleter::operator();

      public:
        UniquePtr() noexcept : m_Data{} {
        }

        UniquePtr(Deleter del) noexcept : m_Data{},
                                          Deleter{ Move(del) } {
        }

        UniquePtr(Pointer ptr) : m_Data{ ptr } {
        }

        UniquePtr(Deleter del, Pointer ptr) : m_Data{ ptr },
                                              Deleter{ Move(del) } {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<UniquePtr, std::decay_t<Args>>::value && ...)>>
        UniquePtr(Args&&... args) : m_Data{ Allocate(Forward<Args>(args)...) } {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<UniquePtr, std::decay_t<Args>>::value && ...)>>
        UniquePtr(Deleter del, Args&&... args) : Deleter{ Move(del) },
                                                 m_Data{ Allocate(Forward<Args>(args)...) } {
        }

        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        UniquePtr(UniquePtr&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        UniquePtr& operator=(UniquePtr&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));

            return *this;
        }

        template <typename T2>
        UniquePtr(UniquePtr<T2>&& rhs) noexcept {
            InitMove<T2>(Move(rhs));
        }

        template <typename T2>
        UniquePtr& operator=(UniquePtr<T2>&& rhs) noexcept {
            Clear();
            InitMove<T2>(Move(rhs));

            return *this;
        }

        ~UniquePtr() {
            Clear();
        }

        [[nodiscard]] Pointer Release() noexcept {
            auto returnValue{ m_Data };
            m_Data = nullptr;
            return returnValue;
        }

        void Delete() noexcept {
            Clear();
        }

        void Reset(Pointer ptr) noexcept {
            Clear();
            m_Data = ptr;
        }

        void Reset(UniquePtr&& rhs) noexcept {
            *this = Move(rhs);
        }

        void Swap(UniquePtr& rhs) noexcept {
            CPointer const data{ m_Data };
            m_Data     = rhs.m_Data;
            rhs.m_Data = data;
        }

        Pointer Get() noexcept {
            return m_Data;
        }

        CPointer Get() const noexcept {
            return m_Data;
        }

        template <IsClass T2 = Type>
        Pointer operator->() noexcept {
            return m_Data;
        }

        template <IsClass T2 = Type>
        CPointer operator->() const noexcept {
            return m_Data;
        }

        Reference operator*() noexcept {
            return *m_Data;
        }

        CReference operator*() const noexcept {
            return *m_Data;
        }

        bool IsValid() const noexcept {
            return m_Data != nullptr;
        }

        explicit operator bool() {
            return m_Data != nullptr;
        }

        explicit operator bool() const {
            return m_Data != nullptr;
        }

      private:
        void InitMove(UniquePtr&& rhs) noexcept {
            m_Data     = rhs.m_Data;
            rhs.m_Data = nullptr;
        }

        template <typename T2>
        void InitMove(UniquePtr<T2>&& rhs) noexcept {
            m_Data     = rhs.m_Data;
            rhs.m_Data = nullptr;
        }

        void Clear() noexcept {
            if (m_Data) {
                (*this)(m_Data);
                m_Data = nullptr;
            }
        }

        template <typename... Args>
        [[nodiscard]] Pointer Allocate(Args&&... args) {
            if constexpr (sizeof...(Args) > 0u) {
                return new Type(Forward<Args>(args)...);
            } else {
                return new Type();
            }
        }

      private:
        Pointer m_Data;
    };

    template <typename T, typename... Args>
    [[nodiscard]] inline UniquePtr<T> MakeUnique(Args&&... args) {
        return UniquePtr<T>{ new T(Forward<Args>(args)...) };
    }
} // namespace adh

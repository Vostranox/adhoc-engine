#pragma once
#include "Concepts.hpp"
#include "UniquePtr.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <atomic>
#include <iostream>

namespace adh {
    namespace _internal {
        struct ControlBlock {
            ControlBlock(std::size_t count, std::ptrdiff_t ptr) : referenceCount{ count },
                                                                  originalPointer{ ptr } {
            }

            std::atomic<std::size_t> referenceCount;
            std::ptrdiff_t originalPointer;
        };
    } // namespace _internal

    template <typename T, typename Deleter = _internal::PointerDefaultDeleter<T>>
    class SharedPtr : Deleter {
      public:
        using Type       = T;
        using Pointer    = Type*;
        using CPointer   = const Type*;
        using Reference  = Type&;
        using CReference = const Type&;

        inline static constexpr std::size_t ZeroCount{ 0u };

      public:
        template <typename T2, typename... Args>
        friend SharedPtr<T2> MakeShared(Args&&... args);
        template <typename, typename>
        friend class SharedPtr;

      private:
        using ControlBlock        = _internal::ControlBlock;
        using ControlBlockPointer = _internal::ControlBlock*;
        using Deleter::operator();

      public:
        SharedPtr() noexcept : m_Data{},
                               m_ControlBlock{} {
        }

        SharedPtr(Deleter del) noexcept : m_Data{},
                                          Deleter{ Move(del) } {
        }

        SharedPtr(Pointer ptr) : m_Data{ ptr } {
            m_ControlBlock = new ControlBlock{ 1u, 0 };
        }

        SharedPtr(Deleter del, Pointer ptr) : m_Data{ ptr },
                                              Deleter{ Move(del) } {
            m_ControlBlock = new ControlBlock{ 1u, 0 };
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<SharedPtr, std::decay_t<Args>>::value && ...)>>
        SharedPtr(Args&&... args) : m_Data{ SmartAllocate(Forward<Args>(args)...) },
                                    m_ControlBlock{ reinterpret_cast<ControlBlockPointer>(m_Data + 1) } {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<SharedPtr, std::decay_t<Args>>::value && ...)>>
        SharedPtr(Deleter del, Args&&... args) : Deleter{ Move(del) },
                                                 m_Data{ SmartAllocate(Forward<Args>(args)...) },
                                                 m_ControlBlock{ reinterpret_cast<ControlBlockPointer>(m_Data + 1) } {
        }

        template <typename T2>
        SharedPtr(UniquePtr<T2>&& uniquePtr) : m_Data{ uniquePtr.m_Data } {
            uniquePtr.m_Data = nullptr;
            m_ControlBlock   = new ControlBlock{ 1u, 0 };
        }

        SharedPtr(const SharedPtr& rhs) {
            InitCopy(rhs);
        }

        SharedPtr& operator=(const SharedPtr& rhs) {
            Clear();
            InitCopy(rhs);

            return *this;
        }

        SharedPtr(SharedPtr&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        SharedPtr& operator=(SharedPtr&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));

            return *this;
        }

        template <typename T2>
        SharedPtr(SharedPtr<T2>&& rhs) noexcept {
            InitMove<T2>(Move(rhs));
        }

        template <typename T2>
        SharedPtr& operator=(SharedPtr<T2>&& rhs) noexcept {
            Clear();
            InitMove<T2>(Move(rhs));

            return *this;
        }

        ~SharedPtr() {
            Clear();
        }

        [[nodiscard]] Pointer Release() noexcept {
            --m_ControlBlock->referenceCount;
            auto returnValue{ m_Data };
            m_Data = nullptr;
            return returnValue;
        }

        void Delete() noexcept {
            Clear();
        }

        void Reset(Pointer ptr) noexcept {
            Clear();
            m_Data         = ptr;
            m_ControlBlock = new ControlBlock{ 1u, 0 };
        }

        void Reset(const SharedPtr& rhs) noexcept {
            *this = rhs;
        }

        void Reset(SharedPtr&& rhs) noexcept {
            *this = Move(rhs);
        }

        void Swap(SharedPtr& rhs) noexcept {
            CPointer const data{ m_Data };
            m_Data     = rhs.m_Data;
            rhs.m_Data = data;

            ControlBlockPointer const block{ m_ControlBlock };
            m_ControlBlock     = rhs.m_ControlBlock;
            rhs.m_ControlBlock = block;
        }

        Pointer Get() noexcept {
            return m_Data;
        }

        CPointer Get() const noexcept {
            return m_Data;
        }

        Reference GetRef() noexcept {
            return *m_Data;
        }

        CReference GetRef() const noexcept {
            return *m_Data;
        }

        std::size_t GetReferenceCount() const noexcept {
            if (m_Data) {
                return m_ControlBlock->referenceCount;
            } else {
                return ZeroCount;
            }
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
        void InitCopy(const SharedPtr& rhs) {
            m_Data         = rhs.m_Data;
            m_ControlBlock = rhs.m_ControlBlock;
            if (m_Data) {
                ++m_ControlBlock->referenceCount;
            }
        }

        void InitMove(SharedPtr&& rhs) noexcept {
            m_Data         = rhs.m_Data;
            m_ControlBlock = rhs.m_ControlBlock;

            rhs.m_Data         = nullptr;
            rhs.m_ControlBlock = nullptr;
        }

        template <typename T2>
        void InitMove(SharedPtr<T2>&& rhs) noexcept {
            m_Data         = rhs.m_Data;
            m_ControlBlock = rhs.m_ControlBlock;

            rhs.m_Data         = nullptr;
            rhs.m_ControlBlock = nullptr;
        }

        void Clear() noexcept {
            if (m_Data) {
                if (!--m_ControlBlock->referenceCount) {
                    if (reinterpret_cast<std::ptrdiff_t>(m_Data) == m_ControlBlock->originalPointer) {
                        m_ControlBlock->~ControlBlock();
                        m_Data->~Type();
                        operator delete(m_Data);
                    } else {
                        (*this)(m_Data);
                        delete m_ControlBlock;
                    }
                }
                m_Data         = nullptr;
                m_ControlBlock = nullptr;
            }
        }

        template <typename... Args>
        [[nodiscard]] Pointer SmartAllocate(Args&&... args) {
            Pointer const raw{ static_cast<Pointer>(operator new(sizeof(Type) + sizeof(ControlBlock))) };
            if constexpr (sizeof...(Args) > 0u) {
                new (raw) Type(Forward<Args>(args)...);
            } else {
                new (raw) Type();
            }

            new (raw + 1) ControlBlock(1u, reinterpret_cast<std::ptrdiff_t>(raw));
            return raw;
        }

      private:
        Pointer m_Data;
        ControlBlockPointer m_ControlBlock;
    };

    template <typename T, typename... Args>
    [[nodiscard]] inline SharedPtr<T> MakeShared(Args&&... args) {
        SharedPtr<T> temp;
        temp.m_Data         = temp.SmartAllocate(Forward<Args>(args)...);
        temp.m_ControlBlock = reinterpret_cast<_internal::ControlBlock*>(temp.m_Data + 1);

        return temp;
    }
} // namespace adh

#pragma once
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
    template <typename T>
    class Stack {
      public:
        using Type      = T;
        using Pointer   = T*;
        using Reference = T&;
        using Bucket    = _internal::Bucket<T>;
        using BucketPtr = Bucket*;
        using SizeType  = std::size_t;

      public:
        Stack() : m_Head{},
                  m_Tail{},
                  m_Bottom{},
                  m_Top{},
                  m_Size{} {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<Stack, std::decay_t<Args>>::value && ...)>>
        Stack(Args&&... args) : m_Head{},
                                m_Tail{},
                                m_Bottom{},
                                m_Top{},
                                m_Size{} {
            (Emplace(Forward<Args>(args)), ...);
        }

        Stack(const Stack& rhs) {
            InitCopy(rhs);
        }

        Stack& operator=(const Stack& rhs) {
            CopyClear();
            InitCopy(rhs);

            return *this;
        }

        Stack(Stack&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        Stack& operator=(Stack&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));

            return *this;
        }

        ~Stack() {
            Clear();
        }

        template <typename... Args>
        decltype(auto) Emplace(Args&&... args) {
            if (!m_Head) {
                Init();
            }
            if (m_Tail->capacity == 0u) {
                EmplaceNextBucket();
            }
            Add(Forward<Args>(args)...);
            return Top();
        }

        Reference Pop() noexcept {
            ADH_THROW(m_Size > 0u, "Popping empty Stack");
            PopBack();
            if (m_Tail->data == m_Top && m_Head != m_Tail) {
                PopNextBucket();
            }
            return Top();
        }

        Reference Bottom() const noexcept {
            return *m_Bottom;
        }

        Reference Top() const noexcept {
            return *(m_Top - 1u);
        }

        SizeType GetSize() const noexcept {
            return m_Size;
        }

        bool IsEmpty() const noexcept {
            return m_Size == 0u;
        }

        void Clear() noexcept {
            while (!IsEmpty()) {
                Pop();
            }
            if (m_Tail) {
                Deallocator(m_Tail);
                m_Head = nullptr;
                m_Tail = nullptr;
            }
        }

      private:
        void Init() {
            m_Head = static_cast<BucketPtr>(Allocator(sizeof(Bucket)));

            m_Tail           = m_Head;
            m_Tail->capacity = Bucket::bucketSize;
            m_Tail->next     = nullptr;

            m_Bottom = m_Tail->data;
            m_Top    = m_Tail->data;
        }

        void InitCopy(const Stack& rhs) {
            if (rhs.m_Head) {
                BucketPtr tail{ rhs.m_Tail };
                Pointer itr{ rhs.m_Tail->data + (Bucket::bucketSize - rhs.m_Tail->capacity) };
                for (SizeType i{}; i != rhs.GetSize(); ++i) {
                    Emplace(*itr--);

                    if (itr == tail->data) {
                        itr  = tail->next->data;
                        tail = tail->next;
                    }
                }
            }
        }

        void CopyClear() {
            while (!IsEmpty()) {
                Pop();
            }
        }

        void InitMove(Stack&& rhs) noexcept {
            std::memcpy(this, &rhs, sizeof(Stack));
            std::memset(&rhs, 0, sizeof(Stack));
        }

        template <typename... Args>
        void Add(Args&&... args) {
            new (m_Top++) Type{ Forward<Args>(args)... };
            --m_Tail->capacity;
            ++m_Size;
        }

        void EmplaceNextBucket() {
            const auto next{ static_cast<BucketPtr>(Allocator(sizeof(Bucket))) };
            next->capacity = Bucket::bucketSize;
            next->next     = m_Tail;

            m_Tail = next;
            m_Top  = m_Tail->data;
        }

        void PopNextBucket() noexcept {
            const auto next{ m_Tail->next };
            Deallocator(m_Tail);
            m_Tail = next;
            m_Top  = m_Tail->data + Bucket::bucketSize;
        }

        void PopBack() noexcept {
            (--m_Top)->~Type();
            --m_Size;
        }

        void* Allocator(SizeType size) {
            return operator new(size);
        }

        void Deallocator(void* ptr) noexcept {
            operator delete(ptr);
        }

      private:
        BucketPtr m_Head;
        BucketPtr m_Tail;
        Pointer m_Bottom;
        Pointer m_Top;
        SizeType m_Size;
    };
} // namespace adh

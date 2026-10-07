#pragma once
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
    template <typename T>
    class Queue {
      public:
        using Type      = T;
        using Pointer   = T*;
        using Reference = T&;
        using Bucket    = _internal::Bucket<T>;
        using BucketPtr = Bucket*;
        using SizeType  = std::size_t;

      public:
        Queue() : m_Head{},
                  m_Tail{},
                  m_Front{},
                  m_Back{},
                  m_Size{} {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<Queue, std::decay_t<Args>>::value && ...)>>
        Queue(Args&&... args) : m_Head{},
                                m_Tail{},
                                m_Front{},
                                m_Back{},
                                m_Size{} {
            (Emplace(Forward<Args>(args)), ...);
        }

        Queue(const Queue& rhs) {
            InitCopy(rhs);
        }

        Queue& operator=(const Queue& rhs) {
            CopyClear();
            InitCopy(rhs);

            return *this;
        }

        Queue(Queue&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        Queue& operator=(Queue&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));

            return *this;
        }

        ~Queue() {
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

            return Back();
        }

        Reference Pop() ADH_NOEXCEPT {
            ADH_THROW(m_Size > 0u, "Popping empty queue");
            PopFront();
            if (m_Head->data + Bucket::bucketSize == m_Front) {
                PopNextBucket();
            }
            return Front();
        }

        Reference Front() const noexcept {
            return *m_Front;
        }

        Reference Back() const noexcept {
            return *(m_Back - 1u);
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

            if (m_Head) {
                Deallocator(m_Head);
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

            m_Front = m_Tail->data;
            m_Back  = m_Tail->data;
        }

        void InitCopy(const Queue& rhs) {
            if (rhs.m_Head) {
                BucketPtr head{ rhs.m_Head };
                Pointer itr{ rhs.m_Head->data };
                for (SizeType i{}; i != rhs.GetSize(); ++i) {
                    Emplace(*itr++);

                    if (itr == head->data + Bucket::bucketSize) {
                        itr  = head->next->data;
                        head = head->next;
                    }
                }
            }
        }

        void CopyClear() {
            while (!IsEmpty()) {
                Pop();
            }
        }

        void InitMove(Queue&& rhs) noexcept {
            std::memcpy(this, &rhs, sizeof(Queue));
            std::memset(&rhs, 0, sizeof(Queue));
        }

        template <typename... Args>
        void Add(Args&&... args) {
            new (m_Back++) Type{ Forward<Args>(args)... };
            --m_Tail->capacity;
            ++m_Size;
        }

        void EmplaceNextBucket() {
            m_Tail->next           = static_cast<BucketPtr>(Allocator(sizeof(Bucket)));
            m_Tail->next->capacity = Bucket::bucketSize;
            m_Tail->next->next     = nullptr;

            m_Tail = m_Tail->next;
            m_Back = m_Tail->data;
        }

        void PopNextBucket() noexcept {
            const auto next{ m_Head->next };
            Deallocator(m_Head);
            m_Head  = next;
            m_Front = m_Head->data;
        }

        void PopFront() noexcept {
            m_Front->~Type();
            ++m_Front;
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
        Pointer m_Front;
        Pointer m_Back;
        SizeType m_Size;
    };
} // namespace adh

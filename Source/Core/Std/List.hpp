#pragma once
#include "Concepts.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
    namespace _internal {
        template <typename T>
        class Node {
          public:
            using Type    = T;
            using NodePtr = Node*;

          public:
            template <typename... Args>
            Node(NodePtr next, NodePtr prev, Args&&... args) : next{ next }, prev{ prev }, data{ Forward<Args>(args)... } {
            }

          public:
            NodePtr next;
            NodePtr prev;
            Type data;
        };

        template <typename T>
        class ListIterator {
          public:
            using Type          = T;
            using CType         = const T;
            using Reference     = T&;
            using CReference    = const T&;
            using Pointer       = T*;
            using CPointer      = const T*;
            using Node          = _internal::Node<Type>;
            using NodePtr       = Node*;
            using Iterator      = ListIterator;
            using ConstIterator = const ListIterator;

          public:
            ListIterator(NodePtr ptr) : m_Data{ ptr } {
            }

            Iterator& operator++() noexcept {
                m_Data = m_Data->next;
                return *this;
            }

            Iterator operator++(int) noexcept {
                Iterator temp{ *this };
                m_Data = m_Data->next;
                return temp;
            }

            ConstIterator& operator++() const noexcept {
                m_Data = m_Data->next;
                return *this;
            }

            ConstIterator operator++(int) const noexcept {
                ConstIterator temp{ *this };
                m_Data = m_Data->next;
                return temp;
            }

            Iterator& operator--() noexcept {
                m_Data = m_Data->prev;
                return *this;
            }

            Iterator operator--(int) noexcept {
                Iterator temp{ *this };
                m_Data = m_Data->prev;
                return temp;
            }

            ConstIterator& operator--() const noexcept {
                m_Data = m_Data->prev;
                return *this;
            }

            ConstIterator operator--(int) const noexcept {
                ConstIterator temp{ *this };
                m_Data = m_Data->prev;
                return temp;
            }

            Reference operator*() noexcept {
                return m_Data->data;
            }

            Pointer operator->() noexcept {
                return &m_Data->data;
            }

            CReference operator*() const noexcept {
                return m_Data->data;
            }

            CPointer operator->() const noexcept {
                return &m_Data->data;
            }

            bool operator==(const Iterator& rhs) const {
                return m_Data == rhs.m_Data;
            }

            bool operator!=(const Iterator& rhs) const {
                return !(*this == rhs);
            }

          public:
            NodePtr m_Data;
        };
    } // namespace _internal

    template <typename T>
    class List {
      public:
        using Type          = T;
        using Node          = _internal::Node<Type>;
        using NodePtr       = _internal::Node<Type>*;
        using Iterator      = _internal::ListIterator<Type>;
        using ConstIterator = const _internal::ListIterator<Type>;
        using SizeType      = std::size_t;

      public:
        List() : m_Head{ static_cast<NodePtr>(Allocator(sizeof(Node))) },
                 m_Tail{ static_cast<NodePtr>(Allocator(sizeof(Node))) } {
            Init();
        }

        template <typename... Args>
        List(Args&&... args) : m_Head{ static_cast<NodePtr>(Allocator(sizeof(Node))) },
                               m_Tail{ static_cast<NodePtr>(Allocator(sizeof(Node))) } {
            Init();
            (EmplaceBack(Forward<Args>(args)), ...);
        }

        List(const List& rhs) : m_Head{ static_cast<NodePtr>(Allocator(sizeof(Node))) },
                                m_Tail{ static_cast<NodePtr>(Allocator(sizeof(Node))) } {
            Init();
            InitCopy(rhs);
        }

        List& operator=(const List& rhs) {
            Clear();
            InitCopy(rhs);

            return *this;
        }

        List(List&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        List& operator=(List&& rhs) noexcept {
            Uninit();
            InitMove(Move(rhs));

            return *this;
        }

        ~List() {
            Uninit();
        }

        template <typename... Args>
        Iterator EmplaceBack(Args&&... args) {
            NodePtr const temp{ new Node{ m_Tail, m_Tail->prev, Forward<Args>(args)... } };
            m_Tail->prev->next = temp;
            m_Tail->prev       = temp;
            ++m_Size;
            return temp;
        }

        template <typename... Args>
        Iterator EmplaceFront(Args&&... args) {
            NodePtr const temp{ new Node{ m_Head->next, m_Head, Forward<Args>(args)... } };
            m_Head->next->prev = temp;
            m_Head->next       = temp;
            ++m_Size;
            return temp;
        }

        void PopFront() noexcept {
            ADH_THROW(m_Size > 0u, "Popping empty list!");

            NodePtr const temp{ m_Head->next };
            m_Head->next->next->prev = m_Head;
            m_Head->next             = m_Head->next->next;

            DeleteNode(temp);
        }

        void PopBack() noexcept {
            ADH_THROW(m_Size > 0u, "Popping empty list!");

            NodePtr const temp{ m_Tail->prev };
            m_Tail->prev->prev->next = m_Tail;
            m_Tail->prev             = m_Tail->prev->prev;

            DeleteNode(temp);
        }

        template <typename... Args>
        Iterator InsertBefore(const Iterator& itr, Args&&... args) {
            ASSERT(itr.m_Data != m_Head, "Inserting before beginning!");
            NodePtr const temp{ new Node{ itr.m_Data, itr.m_Data->prev, Forward<Args>(args)... } };
            itr.m_Data->prev->next = temp;
            itr.m_Data->prev       = temp;
            ++m_Size;
            return temp;
        }

        template <typename... Args>
        Iterator InsertAfter(Iterator& itr, Args&&... args) {
            return InsertBefore(++itr, Forward<Args>(args)...);
        }

        template <typename... Args>
        Iterator InsertAfter(Iterator&& itr, Args&&... args) {
            return InsertBefore(++itr, Forward<Args>(args)...);
        }

        Iterator Erase(const Iterator& itr) noexcept {
            Iterator temp{ itr.m_Data->next };

            itr.m_Data->next->prev = itr.m_Data->prev;
            itr.m_Data->prev->next = itr.m_Data->next;

            DeleteNode(itr.m_Data);

            return temp;
        }

        void Reverse() noexcept {
            NodePtr current{ m_Head };

            while (true) {
                NodePtr next{ current->next };
                current->next = current->prev;
                current->prev = next;
                if (next == m_Head) {
                    m_Tail = m_Head;
                    m_Head = current;
                    break;
                }
                current = next;
            }
        }

        template <IsCallable T2 = Lesser<Type>>
        void Sort() noexcept {
            MergeSort<T2>(m_Head->next, m_Size);
        }

        template <typename... Args>
        void Resize(SizeType newSize, Args&&... args) {
            if (m_Size == newSize) {
                return;
            }

            else if (m_Size < newSize) {
                while (m_Size < newSize) {
                    EmplaceBack(Forward<Args>(args)...);
                }
            }

            else if (m_Size > newSize) {
                while (m_Size > newSize) {
                    PopBack();
                }
            }
        }

        SizeType GetSize() const noexcept {
            return m_Size;
        }

        void Clear() noexcept {
            for (NodePtr itr{ m_Head->next }; itr != m_Tail;) {
                NodePtr const next{ itr->next };

                DeleteNode(itr);

                itr = next;
            }

            Init();
        }

        Iterator begin() noexcept {
            return Iterator{ m_Head->next };
        }

        Iterator end() noexcept {
            return Iterator{ m_Tail };
        }

        ConstIterator begin() const noexcept {
            return Iterator{ m_Head->next };
        }

        ConstIterator end() const noexcept {
            return Iterator{ m_Tail };
        }

      private:
        void Init() {
            m_Head->next = m_Tail;
            m_Head->prev = m_Tail;

            m_Tail->next = m_Head;
            m_Tail->prev = m_Head;
            m_Size       = 0u;
        }

        void InitCopy(const List& rhs) {
            for (auto&& i : rhs) {
                EmplaceBack(i);
            }
        }

        void InitMove(List&& rhs) noexcept {
            std::memcpy(static_cast<void*>(this), &rhs, sizeof(List));
            std::memset(static_cast<void*>(&rhs), 0, sizeof(List));
        }

        void Uninit() noexcept {
            if (m_Head) {
                Clear();
                Deallocator(m_Head);
                Deallocator(m_Tail);
            }
        }

        void DeleteNode(NodePtr ptr) noexcept {
            ptr->~Node();
            Deallocator(ptr);
            --m_Size;
        }

        void* Allocator(std::size_t size) {
            return operator new(size);
        }

        void Deallocator(NodePtr ptr) noexcept {
            operator delete(ptr);
        }

        template <IsCallable T2>
        NodePtr MergeSort(NodePtr first, SizeType size) noexcept {
            if (size < 2u) {
                return first;
            }
            NodePtr second{ Splice(first, size / 2u) };
            first  = MergeSort<T2>(first, size / 2u);
            second = MergeSort<T2>(second, size - size / 2u);

            return Merge<T2>(first, second);
        }

        NodePtr Splice(NodePtr first, SizeType size) noexcept {
            for (SizeType i{}; i != size; ++i) {
                first = first->next;
            }
            first->prev->next = m_Tail;
            return first;
        }

        template <IsCallable T2>
        NodePtr Merge(NodePtr first, NodePtr second) noexcept {
            NodePtr dummy{ m_Head };
            NodePtr dummy_2{ dummy };

            while (first != m_Tail && second != m_Tail) {
                if (T2{}(first->data, second->data)) {
                    Append(dummy, first);
                } else {
                    Append(dummy, second);
                }
            }
            while (first != m_Tail) {
                Append(dummy, first);
            }
            while (second != m_Tail) {
                Append(dummy, second);
            }
            m_Tail->prev = dummy;
            return dummy_2->next;
        }

        void Append(NodePtr& dummy, NodePtr& node) noexcept {
            dummy->next       = node;
            dummy->next->prev = dummy;
            dummy             = dummy->next;
            node              = node->next;
        }

      public:
        NodePtr m_Head;
        NodePtr m_Tail;
        SizeType m_Size;
    };
} // namespace adh

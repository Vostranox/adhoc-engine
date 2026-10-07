#pragma once
#include "Algorithms.hpp"
#include "Array.hpp"
#include "Concepts.hpp"
#include "Iterator.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>

namespace adh {
    template <typename T, IsCallable Order = Greater<T>>
    class PriorityQueue {
      public:
        using Type          = T;
        using Pointer       = Type*;
        using Reference     = Type&;
        using CReference    = const Type&;
        using SizeType      = std::size_t;
        using Iterator      = adh::Iterator<T>;
        using ConstIterator = const Iterator;
        using Container     = Array<Type>;

      public:
        PriorityQueue() : m_Data{} {
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<PriorityQueue, std::decay_t<Args>>::value && ...)>>
        PriorityQueue(Args&&... args) : m_Data{ Forward<Args>(args)... } {
            MakeHeap<Order>(m_Data.begin(), m_Data.end());
        }

        PriorityQueue(const Container& data) : m_Data{ data } {
            MakeHeap<Order>(m_Data.begin(), m_Data.end());
        }

        PriorityQueue(Container&& data) : m_Data{ Move(data) } {
            MakeHeap<Order>(m_Data.begin(), m_Data.end());
        }

        PriorityQueue(const PriorityQueue& rhs) {
            *this = rhs;
        }

        PriorityQueue& operator=(const PriorityQueue& rhs) {
            m_Data = rhs.m_Data;
        }

        PriorityQueue(PriorityQueue&& rhs) noexcept {
            *this = Move(rhs);
        }

        PriorityQueue& operator=(PriorityQueue&& rhs) noexcept {
            m_Data = Move(rhs.m_Data);
        }

        ~PriorityQueue() = default;

        Reference GetTop() noexcept {
            ADH_THROW(m_Data.GetSize() > 0, "Empty Queue!");
            return m_Data[0];
        }

        CReference GetTop() const noexcept {
            ADH_THROW(m_Data.GetSize() > 0, "Empty Queue!");
            return m_Data[0];
        }

        void Pop() noexcept {
            ADH_THROW(m_Data.GetSize() > 0, "Empty Queue!");
            m_Data[0] = Move(m_Data[GetSize() - 1u]);
            m_Data.PopBack();
            PercolateDown<Order, Container, SizeType>(m_Data, m_Data.GetSize(), 1u);
        }

        template <typename... Args>
        Reference Emplace(Args&&... args) {
            auto& temp{ m_Data.EmplaceBack(Forward<Args>(args)...) };
            PercolateUp<Order>(m_Data, m_Data.GetSize());
            return temp;
        }

        void IncreaseKey(SizeType index, const Type& newKey) {
            ADH_THROW(index < m_Data.GetSize(), "Index out of Range!");
            ADH_THROW(m_Data[index] < newKey, "New key is smaller!");
            m_Data[index] = newKey;
            PercolateUp<Order>(m_Data, index + 1u);
        }

        void IncreaseKey(SizeType index, Type&& newKey) {
            ADH_THROW(index < m_Data.GetSize(), "Index out of Range!");
            ADH_THROW(m_Data[index] < newKey, "New key is smaller!");
            m_Data[index] = Move(newKey);
            PercolateUp<Order>(m_Data, index + 1u);
        }

        void DecreaseKey(SizeType index, const Type& newKey) {
            ADH_THROW(index < m_Data.GetSize(), "Index out of Range!");
            ADH_THROW(m_Data[index] > newKey, "New key is bigger!");
            m_Data[index] = newKey;
            PercolateDown<Order, Container, SizeType>(m_Data, m_Data.GetSize(), index + 1u);
        }

        void DecreaseKey(SizeType index, Type&& newKey) {
            ADH_THROW(index < m_Data.GetSize(), "Index out of Range!");
            ADH_THROW(m_Data[index] > newKey, "New key is bigger!");
            m_Data[index] = Move(newKey);
            PercolateDown<Order, Container, SizeType>(m_Data, m_Data.GetSize(), index + 1u);
        }

        bool IsEmpty() const noexcept {
            return m_Data.GetSize() == 0u;
        }

        Pointer GetData() const noexcept {
            return m_Data.GetData();
        }

        SizeType GetSize() const noexcept {
            return m_Data.GetSize();
        }

        SizeType GetCapacity() const noexcept {
            return m_Data.GetCapacity();
        }

        Iterator begin() noexcept {
            return m_Data.begin();
        }

        Iterator end() noexcept {
            return m_Data.end();
        }

        ConstIterator begin() const noexcept {
            return m_Data.begin();
        }

        ConstIterator end() const noexcept {
            return m_Data.end();
        }

      private:
        Container m_Data;
    };
} // namespace adh

#pragma once
#include <Utility.hpp>

#include <iostream>

namespace adh {
    template <typename T>
    class Iterator {
      public:
        using Type          = T;
        using CType         = const T;
        using Reference     = T&;
        using CReference    = const T&;
        using Pointer       = T*;
        using CPointer      = const T*;
        using ConstIterator = const Iterator;
        using SizeType      = std::size_t;

      public:
        Iterator(Pointer ptr) : m_Data{ ptr } {
        }

        Iterator& operator++() noexcept {
            ++m_Data;
            return *this;
        }

        Iterator operator++(int) noexcept {
            Iterator temp{ *this };
            ++m_Data;
            return temp;
        }

        ConstIterator& operator++() const noexcept {
            ++m_Data;
            return *this;
        }

        ConstIterator operator++(int) const noexcept {
            ConstIterator temp{ *this };
            ++m_Data;
            return temp;
        }

        Iterator& operator--() noexcept {
            --m_Data;
            return *this;
        }

        Iterator operator--(int) noexcept {
            Iterator temp{ *this };
            --m_Data;
            return temp;
        }

        ConstIterator& operator--() const noexcept {
            --m_Data;
            return *this;
        }

        ConstIterator operator--(int) const noexcept {
            ConstIterator temp{ *this };
            --m_Data;
            return temp;
        }

        Reference operator*() noexcept {
            return *m_Data;
        }

        Pointer operator->() noexcept {
            return m_Data;
        }

        CReference operator*() const noexcept {
            return *m_Data;
        }

        CPointer operator->() const noexcept {
            return m_Data;
        }

        bool operator==(const Iterator& rhs) const {
            return m_Data == rhs.m_Data;
        }

        bool operator!=(const Iterator& rhs) const {
            return !(*this == rhs);
        }

        Iterator operator+(SizeType offset) noexcept {
            return m_Data + offset;
        }

        ConstIterator operator+(SizeType offset) const noexcept {
            return m_Data + offset;
        }

        Iterator operator-(SizeType offset) noexcept {
            return m_Data - offset;
        }

        ConstIterator operator-(SizeType offset) const noexcept {
            return m_Data - offset;
        }

        auto operator-(const Iterator& rhs) const noexcept {
            return m_Data - rhs.m_Data;
        }

      public:
        mutable Pointer m_Data;
    };
} // namespace adh

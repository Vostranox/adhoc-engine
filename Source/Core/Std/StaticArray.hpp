#pragma once
#include "Algorithms.hpp"
#include "Concepts.hpp"
#include "Iterator.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
    template <typename T, std::size_t Size>
    class StaticArray {
      public:
        using Type          = T;
        using Pointer       = Type*;
        using CPointer      = const Type*;
        using Reference     = Type&;
        using CReference    = const Type&;
        using SizeType      = std::size_t;
        using Iterator      = adh::Iterator<T>;
        using ConstIterator = const Iterator;

      public:
        Pointer GetData() noexcept {
            return m_Data;
        }

        CPointer GetData() const noexcept {
            return m_Data;
        }

        constexpr SizeType GetSize() const noexcept {
            return Size;
        }

        constexpr bool IsEmpty() const noexcept {
            return GetSize() == 0u;
        }

        void Fill(const Type& value) {
            if (sizeof(Type) == sizeof(char)) {
                std::memset(m_Data, static_cast<int>(value), GetSize());
            } else {
                for (SizeType i{}; i != GetSize(); ++i) {
                    m_Data[i] = value;
                }
            }
        }

        template <IsCallable Order = Lesser<Type>>
        void Sort() noexcept {
            return adh::Sort<Order>(begin(), end());
        }

        Reference operator[](SizeType index) noexcept {
            return m_Data[index];
        }

        constexpr CReference operator[](SizeType index) const noexcept {
            return m_Data[index];
        }

        Iterator begin() noexcept {
            return m_Data;
        }

        Iterator end() noexcept {
            return m_Data + GetSize();
        }

        ConstIterator begin() const noexcept {
            return m_Data;
        }

        ConstIterator end() const noexcept {
            return m_Data + GetSize();
        }

      public:
        Type m_Data[Size ? Size : 1u];
    };
} // namespace adh

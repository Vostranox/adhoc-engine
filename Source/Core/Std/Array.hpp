#pragma once
#include "Algorithms.hpp"
#include "Concepts.hpp"
#include "Iterator.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
    template <typename T, typename Deleter = _internal::ArrayDefaultDeleter<T>>
    class Array : Deleter {
      public:
        using Type          = T;
        using Pointer       = Type*;
        using Reference     = Type&;
        using CReference    = const Type&;
        using SizeType      = std::size_t;
        using Iterator      = adh::Iterator<T>;
        using ConstIterator = const Iterator;

      public:
        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<Array, std::decay_t<Args>>::value && ...)>>
        Array(Args&&... args) : m_Data{ nullptr },
                                m_Capacity{ sizeof...(args) },
                                m_Size{ 0u } {
            Init(Forward<Args>(args)...);
        }

        template <typename... Args, typename = std::enable_if_t<(!std::is_base_of<Array, std::decay_t<Args>>::value && ...)>>
        Array(Deleter del, Args&&... args) : Deleter{ Move(del) },
                                             m_Data{ nullptr },
                                             m_Capacity{ sizeof...(args) },
                                             m_Size{ 0u } {
            Init(Forward<Args>(args)...);
        }

        Array(const Array& rhs) {
            InitCopy(rhs);
        }

        Array& operator=(const Array& rhs) {
            Clear();
            InitCopy(rhs);
            return *this;
        }

        Array(Array&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        Array& operator=(Array&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));
            return *this;
        }

        ~Array() {
            Clear();
        }

        template <typename... Args>
        decltype(auto) EmplaceBack(Args&&... args) {
            if ((m_Size + 1u) > m_Capacity) {
                Reallocate(m_Capacity + ((m_Capacity / 2u) + 1u));
            }
            if constexpr (sizeof...(Args) > 0u) {
                new (m_Data + m_Size) Type(Forward<Args>(args)...);
            } else {
                new (m_Data + m_Size) Type();
            }
            return m_Data[m_Size++];
        }

        void PopBack() ADH_NOEXCEPT {
            ADH_THROW(m_Size > 0u, "Popping empty array!");
            --m_Size;
            if (!std::is_trivially_copyable<Type>()) {
                m_Data[m_Size].~Type();
            }
        }

        template <IsIterator Itr, typename... Args>
        void Insert(const Itr& itr, Args&&... args) {
            const std::int64_t index{ static_cast<std::int64_t>(m_Size - (end().m_Data - itr.m_Data)) };
            ADH_THROW(index >= 0, "Inserting before begin!");
            ADH_THROW(index <= static_cast<std::int64_t>(m_Size), "Inserting after end!");
            if (++m_Size > m_Capacity) {
                Reallocate(m_Capacity + ((m_Capacity / 2u) + 1u));
            }
            if (std::is_trivially_copyable<Type>()) {
                std::memcpy(m_Data + index + 1, m_Data + index, sizeof(Type) * (m_Size - index - 1u));
            } else {
                for (auto i{ static_cast<std::int32_t>(m_Size - 2u) }; i >= index; --i) {
                    m_Data[i + 1u] = Move(m_Data[i]);
                }
            }
            new (m_Data + index) Type{ Forward<Args>(args)... };
        }

        template <IsIterator Itr>
        void Erase(const Itr& itr) ADH_NOEXCEPT {
            const std::int64_t index{ static_cast<std::int64_t>(m_Size - (end().m_Data - itr.m_Data)) };
            ADH_THROW(index >= 0, "Erasing before begin!");
            ADH_THROW(index < static_cast<std::int64_t>(m_Size), "Erasing after end!");
            if constexpr (std::is_trivially_copyable<Type>()) {
                std::memmove(m_Data + index, m_Data + index + 1, sizeof(Type) * (m_Size - index - 1u));
            } else {
                for (auto i{ static_cast<std::int32_t>(index) }; i != static_cast<std::int32_t>(m_Size - 1u); ++i) {
                    m_Data[i] = Move(m_Data[i + 1u]);
                }
                m_Data[m_Size - 1u].~Type();
            }
            --m_Size;
        }

        template <typename... Args>
        void Resize(SizeType newSize, Args&&... args) {
            if (newSize > m_Size) {
                if (newSize > m_Capacity) {
                    Reallocate(newSize);
                }
                auto oldSize{ m_Size };
                m_Size = newSize;
                for (SizeType i{ oldSize }; i != m_Size; ++i) {
                    if constexpr (sizeof...(Args) > 0u) {
                        new (m_Data + i) Type(Forward<Args>(args)...);
                    } else {
                        new (m_Data + i) Type();
                    }
                }
            }
        }

        void Reserve(SizeType newCapacity) {
            if (newCapacity > m_Capacity) {
                m_Capacity = newCapacity;
                Reallocate(m_Capacity);
            }
        }

        bool IsEmpty() const noexcept {
            return m_Size == 0u;
        }

        void Fill(const Type& value) {
            if (m_Size > 0u) {
                if (sizeof(Type) == sizeof(char)) {
                    std::memset(m_Data, value, m_Size);
                } else {
                    for (SizeType i{}; i != m_Size; ++i) {
                        m_Data[i] = value;
                    }
                }
            }
        }

        void Clear() noexcept {
            if (m_Data) {
                Deallocate();
            }
        }

        template <typename Order = Lesser<Type>>
        void Sort() noexcept {
            return adh::Sort<Order>(begin(), end());
        }

        Pointer GetData() const noexcept {
            return m_Data;
        }

        SizeType GetSize() const noexcept {
            return m_Size;
        }

        SizeType GetCapacity() const noexcept {
            return m_Capacity;
        }

        Reference operator[](SizeType index) ADH_NOEXCEPT {
            ADH_THROW(index < m_Size, "Array index ouy of range!");
            return m_Data[index];
        }

        CReference operator[](SizeType index) const ADH_NOEXCEPT {
            ADH_THROW(index < m_Size, "Array index ouy of range!");
            return m_Data[index];
        }

        Iterator begin() noexcept {
            return m_Data;
        }

        Iterator end() noexcept {
            return m_Data + m_Size;
        }

        ConstIterator begin() const noexcept {
            return m_Data;
        }

        ConstIterator end() const noexcept {
            return m_Data + m_Size;
        }

      private:
        template <typename... Args>
        void Init(Args&&... args) {
            if (m_Capacity > 0u) {
                Allocate();
                (EmplaceBack(Forward<Args>(args)), ...);
            }
        }

        void InitCopy(const Array& rhs) {
            m_Capacity = rhs.m_Capacity;
            m_Size     = rhs.m_Size;
            Allocate();
            for (SizeType i{}; i != m_Size; ++i) {
                new (m_Data + i) Type(rhs.m_Data[i]);
            }
        }

        void InitMove(Array&& rhs) noexcept {
            std::memcpy(static_cast<void*>(this), &rhs, sizeof(Array));
            std::memset(static_cast<void*>(&rhs), 0, sizeof(Array));
        }

        void Allocate() {
            m_Data = static_cast<Pointer>(operator new(sizeof(Type) * m_Capacity));
            std::memset(static_cast<void*>(m_Data), 0, sizeof(Type) * m_Capacity);
        }

        void Reallocate(SizeType newCapacity) {
            m_Capacity = newCapacity;
            Pointer temp{ static_cast<Pointer>(operator new(sizeof(Type) * m_Capacity)) };
            std::memset(static_cast<void*>(temp), 0, sizeof(Type) * m_Capacity);
            if (m_Data) {
                if constexpr (std::is_trivially_copyable<Type>()) {
                    std::memcpy(temp, m_Data, sizeof(Type) * m_Size);
                } else {
                    for (SizeType i{}; i != m_Size; ++i) {
                        new (temp + i) Type(Move(m_Data[i]));
                        m_Data[i].~Type();
                    }
                }
            }
            UpdatePointer(temp);
        }

        void Deallocate() noexcept {
            if (!std::is_trivially_copyable<Type>()) {
                for (SizeType i{}; i != m_Size; ++i) {
                    m_Data[i].~Type();
                }
            }
            m_Size     = 0u;
            m_Capacity = 0u;
            UpdatePointer(nullptr);
        }

        void UpdatePointer(Pointer ptr) {
            (*this)(m_Data);
            m_Data = ptr;
        }

      private:
        Pointer m_Data;
        SizeType m_Capacity;
        SizeType m_Size;
    };
} // namespace adh

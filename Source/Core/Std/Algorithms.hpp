#pragma once
#include "Concepts.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>

namespace adh {
    using IndexType = ptrdiff_t;

    namespace _internal {
        // Sort Function
        template <typename Order, typename Ptr>
        inline void Insertion(Ptr* arr, IndexType l, IndexType h) noexcept {
            for (IndexType j{ l + 1 }; j != h; ++j) {
                auto key{ arr[j] };
                IndexType i{ j - 1 };
                for (; i >= 0 && Order{}(key, arr[i]); --i) {
                    arr[i + 1] = arr[i];
                }
                arr[i + 1] = key;
            }
        }

        // Sort Function
        template <typename Order, typename Ptr>
        inline decltype(auto) Median(Ptr* arr, IndexType l, IndexType h) {
            IndexType m{ (l + h) / 2 };
            if (Order{}(arr[m], arr[l])) {
                Swap(arr[m], arr[l]);
            }
            if (Order{}(arr[h], arr[l])) {
                Swap(arr[h], arr[l]);
            }
            if (Order{}(arr[h], arr[m])) {
                Swap(arr[h], arr[m]);
            }
            Swap(arr[m], arr[h - 1]);
            return arr[h - 1];
        }

        // Sort Function
        template <typename Order, typename Ptr>
        inline IndexType Partition(Ptr* arr, IndexType l, IndexType h) {
            auto pivot{ Move(Median<Order>(arr, l, h)) };
            IndexType i{ l };
            IndexType j{ h - 1 };

            while (i < j) {
                while (Order{}(arr[++i], pivot)) {
                }

                while (Order{}(pivot, arr[--j])) {
                }

                if (i < j) {
                    Swap(arr[i], arr[j]);
                }
            }
            Swap(arr[i], arr[h - 1]);
            return i;
        }

        // Sort Function
        template <typename Order, typename Ptr>
        inline void QuickSort(Ptr* arr, IndexType l, IndexType h) {
            if (l + 20u < h) {
                IndexType m{ Partition<Order>(arr, l, h) };
                QuickSort<Order>(arr, l, m - 1);
                QuickSort<Order>(arr, m + 1, h);
            }

            else {
                Insertion<Order>(arr, l, h + 1);
            }
        }

        // Heap Function
        template <typename Order, typename Conteiner, typename Size>
        inline void PercolateDown(Conteiner& arr, Size n, Size i) noexcept {
            Size child;
            auto temp{ Move(arr[i - 1u]) };
            for (; i * 2u <= n; i = child) {
                child = i * 2u;

                if (child != n && Order{}(arr[child], arr[child - 1u])) {
                    ++child;
                }
                if (Order{}(arr[child - 1u], temp)) {
                    arr[i - 1u] = Move(arr[child - 1u]);
                } else {
                    break;
                }
            }
            arr[i - 1u] = Move(temp);
        }

        // Heap Function
        template <typename Order, typename Conteiner, typename Size>
        inline void PercolateUp(Conteiner& arr, Size i) noexcept {
            Size parent;
            auto temp{ Move(arr[i - 1u]) };
            for (; i / 2u > 0u; i = parent) {
                parent = i / 2u;

                if (Order{}(temp, arr[parent - 1u])) {
                    arr[i - 1u] = Move(arr[parent - 1u]);
                } else {
                    break;
                }
            }

            arr[i - 1u] = Move(temp);
        }
    } // namespace _internal

    template <typename Order, IsIterator Itr>
    inline void Sort(const Itr& begin, const Itr& end) noexcept {
        if (begin != end) {
            _internal::QuickSort<Order>(begin.m_Data, 0u, (end - begin) - 1);
        }
    }

    template <typename T, IsIterator Itr>
    inline Itr Find(const Itr& begin, const Itr& end, const T& elem) noexcept {
        for (auto itr{ begin }; itr != end; ++itr) {
            if (*itr == elem) {
                return itr;
            }
        }
        return end;
    }

    template <typename Order, IsIterator Itr>
    inline void MakeHeap(const Itr& begin, const Itr& end) noexcept {
        auto size{ end - begin };
        for (decltype(size) i{ size / 2u }; i != 0u; --i) {
            _internal::PercolateDown<Order>(begin.m_Data, size, i);
        }
    }
} // namespace adh

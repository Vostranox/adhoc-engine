#pragma once
#include <iostream>

namespace adh {
    template <typename T>
    concept Comparable = requires(T x, T y) {
        x > y;
        x < y;

        x >= y;
        x <= y;

        x == y;
        x != y;
    };

    template <typename T>
    concept IsIterator = requires(T t, T t2) {
        ++t;
        t++;
        --t;
        t--;
        *t;
        t == t2;
        t != t2;
        t.operator->();
        t.m_Data;
    };

    template <typename T>
    concept IsCallable = requires(T t) {
        t.operator();
    };

    template <typename T>
    concept IsClass = std::is_class_v<T>;

    template <typename T>
    concept IsVector = requires(T t) {
        t[0];
    };

    template <typename T>
    concept IsMatrix = requires(T t) {
        t[0][0];
    };
} // namespace adh

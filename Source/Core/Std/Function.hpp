#pragma once
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>

namespace adh {
    template <typename T>
    class Function;

    template <typename R, typename... Args>
    class Function<R(Args...)> {
      private:
        using ReturnType        = R;
        using InvokeFuncType    = R (*)(void*, Args&&...);
        using ConstructFuncType = void (*)(void*, void*);
        using DestructFuncType  = void (*)(void*);

      public:
        Function() : m_Data{},
                     m_Size{},
                     m_Invoke{},
                     m_Construct{},
                     m_Destroy{} {
        }

        template <typename Functor>
        Function(Functor func) {
            InitializeConstruct(Move(func));
        }

        Function(const Function& rhs) {
            CopyConstruct(rhs);
        }

        Function& operator=(const Function& rhs) {
            Clear();
            CopyConstruct(rhs);

            return *this;
        }

        Function(Function&& rhs) noexcept {
            MoveConstruct(Move(rhs));
        }

        Function& operator=(Function&& rhs) noexcept {
            Clear();
            MoveConstruct(Move(rhs));

            return *this;
        }

        ~Function() {
            Clear();
        }

        ReturnType operator()(Args... args) noexcept {
            return m_Invoke(m_Data, Forward<Args>(args)...);
        }

        operator bool() const noexcept {
            return m_Data;
        }

        template <typename Functor>
        void operator=(Functor func) {
            Clear();
            InitializeConstruct(Move(func));
        }

        void operator=(std::nullptr_t) noexcept {
            Clear();
        }

      private:
        template <typename Functor>
        void InitializeConstruct(Functor func) {
            m_Data      = new char[sizeof(Functor)];
            m_Size      = sizeof(Functor);
            m_Invoke    = Invoke<Functor>;
            m_Construct = Construct<Functor>;
            m_Destroy   = Destroy<Functor>;

            m_Construct(m_Data, reinterpret_cast<char*>(&func));
        }

        void CopyConstruct(const Function& rhs) {
            m_Size      = rhs.m_Size;
            m_Invoke    = rhs.m_Invoke;
            m_Construct = rhs.m_Construct;
            m_Destroy   = rhs.m_Destroy;

            m_Data = new char[m_Size];
            m_Construct(m_Data, rhs.m_Data);
        }

        void MoveConstruct(Function&& rhs) noexcept {
            m_Size      = rhs.m_Size;
            m_Invoke    = rhs.m_Invoke;
            m_Construct = rhs.m_Construct;
            m_Destroy   = rhs.m_Destroy;
            m_Data      = rhs.m_Data;

            rhs.m_Data = nullptr;
        }

        void Clear() noexcept {
            if (m_Data) {
                m_Destroy(m_Data);
                delete[] m_Data;
                m_Data = nullptr;
            }
        }

      private:
        template <typename Functor>
        static ReturnType Invoke(void* func, Args&&... args) noexcept {
            return (*static_cast<Functor*>(func))(Forward<Args>(args)...);
        }

        template <typename Functor>
        static void Construct(void* lhs, void* rhs) {
            new (lhs) Functor(*static_cast<Functor*>(rhs));
        }

        template <typename Functor>
        static void Destroy(void* func) noexcept {
            static_cast<Functor*>(func)->~Functor();
        }

      private:
        char* m_Data;
        std::size_t m_Size;
        InvokeFuncType m_Invoke;
        ConstructFuncType m_Construct;
        DestructFuncType m_Destroy;
    };
} // namespace adh

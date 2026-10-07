#pragma once
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <string>

namespace adh {
    class File {
      public:
        enum class Mode {
            Write,
            Read,
            ReadWrite,
            Append,
            WriteBinary,
            ReadBinary,
            ReadWriteBinary,
            AppendBinary,
        };

        enum class Position {
            Begin,
            Current,
            End,
        };

      public:
        File() noexcept : m_File{} {
        }

        File(const char* filePath, Mode mode) noexcept {
            Open(filePath, mode);
        }

        File(const std::string& filePath, Mode mode) noexcept {
            Open(filePath.data(), mode);
        }

        File(const File&) = delete;
        File& operator=(const File&) = delete;
        File(File&&)                 = delete;
        File& operator=(File&&) = delete;

        ~File() {
            Close();
        }

        template <typename T>
        void Read(T&& to, std::size_t size) const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            fread(&to, size, 1u, m_File);
        }

        template <typename T>
        void Read(T* to, std::size_t size) const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            fread(to, size, 1u, m_File);
        }

        template <typename T>
        void Write(T&& from, std::size_t size) const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            fwrite(&from, size, 1u, m_File);
        }

        template <typename T>
        void Write(T* from, std::size_t size) const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            fwrite(from, size, 1u, m_File);
        }

        void Seek(Position origin, std::int32_t offset = 0) const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            fseek(m_File, offset, static_cast<std::int32_t>(origin));
        }

        auto GetSize() const ADH_NOEXCEPT {
            ADH_THROW(m_File != nullptr, "File is null!");
            Seek(Position::End);
            const std::int32_t size{ static_cast<std::int32_t>(ftell(m_File)) };
            Seek(Position::Begin);
            return size;
        }

        void Open(const char* filePath, Mode mode) ADH_NOEXCEPT {
            ADH_THROW(filePath != nullptr, "File path is null!");
            m_File = fopen(filePath, m_Table[static_cast<std::size_t>(mode)]);
            ADH_THROW(m_File != nullptr, "Failed to open file!");
        }

        void Close() const noexcept {
            if (m_File) {
                fclose(m_File);
            }
        }

      private:
        FILE* m_File;

        inline static const char* m_Table[]{
            "w",
            "r",
            "r+",
            "a+",
            "wb",
            "rb",
            "rb+",
            "ab+",
        };
    };
} // namespace adh

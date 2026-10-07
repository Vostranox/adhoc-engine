#pragma once
#include "File.hpp"
#include "Utility.hpp"
#include <Utility.hpp>

#include <iostream>
#include <cstring>

namespace adh {
#pragma pack(push, 1)
    struct TGAHeader {
        char IDLenght;
        char colorMapType;
        char imageType;

        short colorMapOrigin;
        short colorMapLenght;
        char colorMapDepth;

        short xOrigin;
        short yOrigin;
        short width;
        short height;
        char bitsPerPixel;
        char imageDescriptor;
    };
#pragma pack(pop)

    class TGALoader {
      public:
        TGALoader(const char* filePath) {
            TGAHeader header;

            // Read header data
            File file{ filePath, File::Mode::ReadBinary };
            file.Read(header, sizeof(TGAHeader));

            m_Width  = header.width;
            m_Height = header.height;

            // Select rgb or rgba
            switch (header.bitsPerPixel) {
            case 32:
                m_Offset = 4u;
                break;

            case 24:
                m_Offset = 3u;
                break;

            default:
                ADH_THROW(header.bitsPerPixel == 32 || header.bitsPerPixel == 24, "Not a 32 or 24 bit TGA");
                break;
            }

            // Calculate size of image
            m_Size = static_cast<std::size_t>(m_Width * m_Height * m_Offset);

            // Read image data
            m_Data = { static_cast<unsigned char*>(operator new(m_Size)) };
            file.Read(m_Data, m_Size);
        }

        TGALoader(const TGALoader& rhs) {
            InitCopy(rhs);
        }

        TGALoader& operator=(const TGALoader& rhs) {
            Clear();
            InitCopy(rhs);

            return *this;
        }

        TGALoader(TGALoader&& rhs) noexcept {
            InitMove(Move(rhs));
        }

        TGALoader& operator=(TGALoader&& rhs) noexcept {
            Clear();
            InitMove(Move(rhs));

            return *this;
        }

        ~TGALoader() {
            Clear();
        }

        auto GetData() const noexcept {
            return m_Data;
        }

        auto GetSize() const noexcept {
            return m_Size;
        }

        auto GetWidth() const noexcept {
            return m_Width;
        }

        auto GetHeight() const noexcept {
            return m_Height;
        }

        void InvertRedBlue() noexcept {
            for (std::size_t y{}; y != m_Height; ++y) {
                for (std::size_t x{}; x != m_Width; ++x) {
                    const auto index{ (y * m_Width + x) * m_Offset };
                    const auto temp{ m_Data[index] };
                    m_Data[index]     = m_Data[index + 2];
                    m_Data[index + 2] = temp;
                }
            }
        }

        void Clear() noexcept {
            if (m_Data) {
                operator delete(m_Data);
                m_Data   = nullptr;
                m_Width  = 0u;
                m_Height = 0u;
                m_Size   = 0u;
                m_Offset = 0u;
            }
        }

      private:
        void InitCopy(const TGALoader& rhs) {
            m_Data = static_cast<unsigned char*>(operator new(rhs.m_Size));
            std::memcpy(m_Data, rhs.m_Data, rhs.m_Size);

            m_Width  = rhs.m_Width;
            m_Height = rhs.m_Height;
            m_Offset = rhs.m_Offset;
            m_Size   = rhs.m_Size;
        }

        void InitMove(TGALoader&& rhs) {
            m_Data   = rhs.m_Data;
            m_Width  = rhs.m_Width;
            m_Height = rhs.m_Height;
            m_Offset = rhs.m_Offset;
            m_Size   = rhs.m_Size;

            rhs.m_Data   = nullptr;
            rhs.m_Width  = 0u;
            rhs.m_Height = 0u;
            rhs.m_Offset = 0u;
            rhs.m_Size   = 0u;
        }

      private:
        unsigned char* m_Data{};
        std::size_t m_Width{};
        std::size_t m_Height{};
        std::size_t m_Offset{};
        std::size_t m_Size{};
    };
} // namespace adh

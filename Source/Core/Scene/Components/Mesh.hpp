#pragma once
#include <Std/Array.hpp>
#include <Std/SharedPtr.hpp>
#include <Vulkan/IndexBuffer.hpp>
#include <Vulkan/VertexBuffer.hpp>
#include <string>

#include <Vertex.hpp>
#include <unordered_map>

namespace adh {
    struct MeshBufferData {
        vk::IndexBuffer index;
        vk::VertexBuffer vertex;
        Array<Vertex> vertices;
        Array<Vector3D> vertices2;
        Array<std::uint32_t> indices;
    };

    class Mesh {
      public:
        void Bind(VkCommandBuffer commandBuffer) noexcept {
            if (bufferData) {
                bufferData->index.Bind(commandBuffer);
                bufferData->vertex.Bind(commandBuffer);
            }
        }

        std::uint32_t GetIndexCount() const noexcept {
            return bufferData ? bufferData->index.GetCount() : 0u;
        }

        void Load(const std::string& meshPath);

        void Load2(const char* fileName);

        const std::string& GetName() const noexcept {
            return name;
        }

        const std::string& GetFilePath() const noexcept {
            return filePath;
        }

        MeshBufferData* Get() noexcept {
            return bufferData.Get();
        }

        const MeshBufferData* Get() const noexcept {
            return bufferData.Get();
        }

        static void Clear() noexcept {
            meshes.clear();
        }

      public:
        bool toDraw{ true };

      private:
        SharedPtr<MeshBufferData> bufferData;
        std::string name;
        std::string filePath;
        inline static std::unordered_map<std::string, SharedPtr<MeshBufferData>> meshes;
    };
} // namespace adh

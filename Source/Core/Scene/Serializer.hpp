#pragma once
#include <filesystem>
#include <string>

// TODO: efficient load-save from play
namespace adh {
    class Scene;
    class Serializer {

      public:
        Serializer(Scene* scene);

        void Serialize();

        bool SerializeToFile(const std::filesystem::path& filePath);

        void Deserialize();

        bool DeserializeFromFile(const std::filesystem::path& filePath);

        std::string ToText() const;

        void FromText(const std::string& text);

      private:
        void Deserialize(void* pNode);

      private:
        std::string m_Data;
        Scene* m_Scene;
    };
} // namespace adh

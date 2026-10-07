#include "Components.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
    template <>
    struct convert<adh::Vector2D> {
        static Node encode(const adh::Vector2D& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, adh::Vector2D& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template <>
    struct convert<adh::Vector3D> {
        static Node encode(const adh::Vector3D& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, adh::Vector3D& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <>
    struct convert<adh::Vector4D> {
        static Node encode(const adh::Vector4D& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, adh::Vector4D& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template <>
    struct convert<adh::xmm::Vector> {
        static Node encode(const adh::xmm::Vector& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, adh::xmm::Vector& rhs) {
            if (!node.IsSequence()) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
} // namespace YAML

inline YAML::Emitter& operator<<(YAML::Emitter& out, const adh::Vector2D& rhs) {
    out << YAML::Flow;
    out << YAML::BeginSeq << rhs.x << rhs.y << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const adh::Vector3D& rhs) {
    out << YAML::Flow;
    out << YAML::BeginSeq << rhs.x << rhs.y << rhs.z << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const adh::Vector4D& rhs) {
    out << YAML::Flow;
    out << YAML::BeginSeq << rhs.x << rhs.y << rhs.z << rhs.w << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const adh::xmm::Vector& rhs) {
    out << YAML::Flow;
    out << YAML::BeginSeq << rhs.x << rhs.y << rhs.z << rhs.w << YAML::EndSeq;
    return out;
}

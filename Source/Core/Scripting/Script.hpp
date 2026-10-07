#pragma once

#include <adh/script.hpp>

#include <string>
#include <utility>

namespace adh {
    struct Script {
        Script() = default;

        Script(script::State& state, std::string path)
            : filePath{ std::move(path) },
              instance{ state.create_script_file(filePath.c_str()) } {}

        std::string GetFileName() const {
            return filePath.substr(filePath.find_last_of('/') + 1);
        }

        void Compile(script::State& state) {
            if (filePath.empty()) {
                instance = state.create_script(source.c_str());
            } else {
                instance = state.create_script_file(filePath.c_str());
            }
            fixedUpdateAccumulator = {};
        }

        std::string source;
        std::string filePath;
        script::Script instance;
        float fixedUpdateAccumulator{};
    };
} // namespace adh

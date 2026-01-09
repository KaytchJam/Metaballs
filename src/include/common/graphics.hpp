#pragma once

#include "../../dependencies/glm/glm.hpp"

#include <vector>

namespace mbl {
    namespace common {
        namespace graphics {
            /** Simple vertex data type that stores a position and a normal */
            struct Vertex {
                glm::vec3 position;
                glm::vec3 normal;
            };
        
            /** Stores index data and vertices */
            struct MeshData {
                std::vector<Vertex> vertices;
                std::vector<int32_t> indices;
            };
        }
    }
}
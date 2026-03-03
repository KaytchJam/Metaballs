#pragma once

#include <vector>

namespace mbl {
    namespace common {
        namespace graphics {
            /** Simple vertex data type that stores a position and a normal */
            struct Vertex {
                lalg::vec3 position;
                lalg::vec3 normal;
            };
        
            /** Stores index data and vertices */
            struct MeshData {
                std::vector<Vertex> vertices;
                std::vector<int32_t> indices;
            };
        }
    }
}
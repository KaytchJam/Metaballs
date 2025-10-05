#pragma once

#include "dependencies/glm/glm.hpp"

#include <vector>

namespace mball {

    /** Representation of a vertex. */
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };
    
    /** Class that renders multiple metaball instances
     * within a field defined by the user. */
    class MetaballRenderer {
        float iso_value = 1.f;
        std::vector<Vertex> vertices;
    };
}

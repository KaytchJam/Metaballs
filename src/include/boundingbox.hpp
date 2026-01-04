#pragma once

#include "../dependencies/glm/glm.hpp"

/** Simple representation of a 3D Bounding Box with a maximal point & a minimal point */
struct BoundingBox {
    glm::vec3 max_point;
    glm::vec3 min_point;

    BoundingBox& intersection_mut(const BoundingBox& outer) {
        this->min_point = max(min_point, outer.min_point);
        this->max_point = min(max_point, outer.max_point);
        return *this;
    }

    BoundingBox& join_mut(const BoundingBox& other) {
        this->min_point = min(min_point, other.min_point);
        this->max_point = max(max_point, other.max_point);
        return *this;
    }
};

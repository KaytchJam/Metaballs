#pragma once

#include <common/lalg.hpp>

#include <limits>


/** Simple representation of a 3D Bounding Box with a maximal point & a minimal point */
struct BoundingBox {
    gtt::lalg::vec3 max_point = gtt::lalg::vec3(0);
    gtt::lalg::vec3 min_point = gtt::lalg::vec3(0);

    BoundingBox& intersection_mut(const BoundingBox& outer);

    BoundingBox& join_mut(const BoundingBox& other);

    constexpr gtt::lalg::vec3 get_center() const;

    /** Calculate the volume of the Bounding box. */
    float volume() const;

    /** Calculate the surface area of the Bounding box. */
    float surface_area() const;

    /** Returns an 'empty' `BoundingBox`, where its minimum point
     * is positive infinity, and its maximum point is negative
     * infinity. 
     * 
     * One property of this "empty bounding box" is that it
     * acts as the identity element for the join operation,
     * meaning... `join(a, empty()) = a`.
     *  */
    static constexpr BoundingBox empty() {
        return BoundingBox {
            -std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()
        };
    }

    /** Returns a 'universal' `BoundingBox`, where its min point
     * is negative infinity, and its max point is positive
     * infinity.
     * 
     * One property of this "universal bounding box" is that it
     * acts as the absorbing element for the join operation,
     * meaning... `join(a, universal()) = universal()`.
     */
    static constexpr BoundingBox universal() {
        return BoundingBox {
            std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };
    }
};

namespace gtt {
    BoundingBox join(const BoundingBox& b1, const BoundingBox& b2);

    bool overlapping(const BoundingBox& a, const BoundingBox& b);

    BoundingBox expand(const BoundingBox& b, const float constant);

    BoundingBox& expand_mut(BoundingBox& b, const float constant);

    BoundingBox scale(const BoundingBox& b, const float scalar);

    BoundingBox& scale_mut(BoundingBox& b, const float scalar);
}

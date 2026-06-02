#include <boundingbox.hpp>

BoundingBox& BoundingBox::intersection_mut(const BoundingBox& outer) {
    this->min_point = max(min_point, outer.min_point);
    this->max_point = min(max_point, outer.max_point);
    return *this;
}

BoundingBox& BoundingBox::join_mut(const BoundingBox& other) {
    this->min_point = min(min_point, other.min_point);
    this->max_point = max(max_point, other.max_point);
    return *this;
}

constexpr gtt::lalg::vec3 BoundingBox::get_center() const {
    return ( max_point + min_point ) * 0.5f;
}

float BoundingBox::volume() const {
    return fold(max_point - min_point, 1.f, [](const float a, const float b) { 
        return std::abs(a * b); 
    });
}

float BoundingBox::surface_area() const {
    gtt::lalg::vec3 dimensions = map(max_point - min_point, [](const float a) { return std::abs(a); });
    return 2 * (dimensions.x * dimensions.y + dimensions.y * dimensions.z + dimensions.z * dimensions.x );
}

namespace gtt {
    BoundingBox join(const BoundingBox& b1, const BoundingBox& b2) {
        return BoundingBox(
            min(b1.min_point, b2.min_point),
            max(b1.max_point, b2.max_point)
        );
    }

    bool overlapping(const BoundingBox& a, const BoundingBox& b) {
        return !(
            (a.max_point.x < b.min_point.x || b.max_point.x < a.min_point.x) || 
            (a.max_point.y < b.min_point.y || b.max_point.y < a.min_point.y) || 
            (a.max_point.z < b.min_point.z || b.max_point.z < a.min_point.z)
        );
    }

    BoundingBox expand(const BoundingBox& b, const float constant) {
        const lalg::vec3 constant_vec(constant);
        return BoundingBox(
            b.min_point - constant_vec,
            b.max_point + constant_vec
        );
    }

    BoundingBox& expand_mut(BoundingBox& b, const float constant) {
        const lalg::vec3 constant_vec(constant);
        b.max_point += constant_vec;
        b.min_point -= constant_vec;
        return b;
    }

    BoundingBox scale(const BoundingBox& b, const float scalar) {
        const float diagonal = lalg::distance(b.max_point, b.min_point);
        return expand(b, diagonal * scalar - diagonal);
    }

    BoundingBox& scale_mut(BoundingBox& b, const float scalar) {
        const float diagonal = lalg::distance(b.max_point, b.min_point);
        return expand_mut(b, diagonal * scalar - diagonal);
    }
}
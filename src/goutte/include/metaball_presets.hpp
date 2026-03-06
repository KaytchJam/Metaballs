#pragma once

#include <metaball.hpp>

namespace gtt {
    /** Pre-defined metaball structs can be found here. */
    namespace presets {
        struct InverseSquareBlob {
            lalg::vec3 m_center = lalg::vec3(0.0f);
            float m_scale = 1.0;

            InverseSquareBlob(const lalg::vec3& center = lalg::vec3(0.0), const float scale = 1.0f) 
                : m_center(center), m_scale(scale) {}

            float operator()(float x, float y, float z) const {
                return m_scale / (
                    (float) std::pow(m_center.x - x, 2) + 
                    (float) std::pow(m_center.y - y, 2) + 
                    (float) std::pow(m_center.z - z, 2)
                );
            }

            BoundingBox get_bounding_box() const {
                const lalg::vec3 sqrt_of_scale_vec(sqrtf(m_scale));
                return BoundingBox{ m_center + sqrt_of_scale_vec, m_center - sqrt_of_scale_vec };
            }
        };

        struct Gaussian {
            float variance = 1.0f;

            float operator()(float x, float y, float z) const {
                return expf(-(x*x + y*y + z*z) / (2*variance));
            }
        };

        struct StickyPlane {
            lalg::vec3 m_center = lalg::vec3(0.f);
            float m_offset = 1.0f;

            StickyPlane(const lalg::vec3& center = lalg::vec3(0.f), const float offset = 1.0f) 
                : m_center(center), m_offset(offset) {}

            float operator()(float x, float y, float z) const {
                return (m_center.x - x) + (m_center.y - y) + (m_center.z - z) + m_offset;
            }
        };

        struct InverseSquareCube {
            lalg::vec3 m_center = lalg::vec3(0.f);
            float m_scale = 1.0f;
            float m_eps = 0.f;

            InverseSquareCube(const lalg::vec3& center = lalg::vec3(0.f), const float scale = 1.0f, const float eps = 0.f) 
                : m_center(center), m_scale(scale), m_eps(eps) {}

            float operator()(float x, float y, float z) const {
                return m_scale / (
                    (float) std::pow(m_center.x - x, 4) +
                    (float) std::pow(m_center.y - y, 4) +
                    (float) std::pow(m_center.z - z, 4) +
                    m_eps
                );
            }
        };

        struct KineticBlob {
            lalg::vec3 m_center = lalg::vec3(0.f);
            lalg::vec3 m_velocity = lalg::vec3(0.f);
            float m_scale = 1.f;

            KineticBlob(const lalg::vec3& center = lalg::vec3(0.0), const lalg::vec3& velocity = lalg::vec3(0.0), const float scale = 1.0f) 
                : m_center(center), m_velocity(velocity), m_scale(scale) {}

            float operator()(float x, float y, float z) const {
                return m_scale / (
                    (float) std::pow(m_center.x - x, 2) + 
                    (float) std::pow(m_center.y - y, 2) + 
                    (float) std::pow(m_center.z - z, 2)
                );
            }

            lalg::vec3& update(const float dt) {
                m_center = m_center + m_velocity * dt;
                return m_center;
            }

            BoundingBox get_bounding_box() const {
                const lalg::vec3 sqrt_of_scale_vec(sqrtf(m_scale));
                return BoundingBox{ m_center + sqrt_of_scale_vec, m_center - sqrt_of_scale_vec };
            }
        };

    }
}
#pragma once

#include <metaball.hpp>

namespace gtt {
    /** Pre-defined metaball structs can be found here. */
    namespace presets {
        struct InverseSquareBlob {
            lalg::vec3 m_center = lalg::vec3(0.0f);
            float m_scale = 1.0;
            float m_influence = 1.2f;

            InverseSquareBlob(const lalg::vec3& center = lalg::vec3(0.0), const float scale = 1.0f, const float influence = 1.5f) 
                : m_center(center), m_scale(scale), m_influence(influence) {}

            float operator()(float x, float y, float z) const {
                const float dx = m_center.x - x;
                const float dy = m_center.y - y;
                const float dz = m_center.z - z;
                return m_scale / (dx*dx + dy*dy + dz*dz);
            }

            BoundingBox get_bounding_box() const {
                const lalg::vec3 sqrt_of_scale_vec(sqrtf(m_scale));
                return BoundingBox{ m_center - sqrt_of_scale_vec, m_center + sqrt_of_scale_vec,  };
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
                const float dx = m_center.x - x;
                const float dy = m_center.y - y;
                const float dz = m_center.z - z;
                return m_scale / (dx*dx*dx*dx + dy*dy*dy*dy + dz*dz*dz*dz + m_eps);
            }
        };

        struct KineticBlob {
            lalg::vec3 m_center = lalg::vec3(0.f);
            lalg::vec3 m_velocity = lalg::vec3(0.f);
            float m_scale = 1.f;
            float m_influence = 1.5f;

            KineticBlob(const lalg::vec3& center = lalg::vec3(0.0), const lalg::vec3& velocity = lalg::vec3(0.0), const float scale = 1.0f, const float influence = 1.5f) 
                : m_center(center), m_velocity(velocity), m_scale(scale), m_influence(influence) {}

            float operator()(float x, float y, float z) const {
                const float dx = m_center.x - x;
                const float dy = m_center.y - y;
                const float dz = m_center.z - z;
                return m_scale / (dx*dx + dy*dy + dz*dz);
            }

            lalg::vec3& update(const float dt) {
                m_center = m_center + m_velocity * dt;
                return m_center;
            }

            BoundingBox get_bounding_box() const {
                const lalg::vec3 sqrt_of_scale_vec(sqrtf(m_scale) * m_influence);
                return BoundingBox{ m_center - sqrt_of_scale_vec, m_center + sqrt_of_scale_vec };
            }
        };

    }
}
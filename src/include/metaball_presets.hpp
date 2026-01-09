#pragma once

#include <metaball.hpp>

namespace mbl {
    /** Pre-defined metaball structs can be found here. */
    namespace presets {
        struct InverseSquareBlob {
            glm::vec3 m_center = glm::vec3(0.0f);
            float m_scale = 1.0;

            InverseSquareBlob(const glm::vec3& center = glm::vec3(0.0), const float scale = 1.0f) 
                : m_center(center), m_scale(scale) {}

            float operator()(float x, float y, float z) const {
                return m_scale / (
                    (float) std::pow(m_center.x - x, 2) + 
                    (float) std::pow(m_center.y - y, 2) + 
                    (float) std::pow(m_center.z - z, 2)
                );
            }

            BoundingBox get_bounding_box() const {
                const glm::vec3 sqrt_of_scale_vec(sqrtf(m_scale));
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
            glm::vec3 m_center = glm::vec3(0.f);
            float m_offset = 1.0f;

            StickyPlane(const glm::vec3& center = glm::vec3(0.f), const float offset = 1.0f) 
                : m_center(center), m_offset(offset) {}

            float operator()(float x, float y, float z) const {
                return (m_center.x - x) + (m_center.y - y) + (m_center.z - z) + m_offset;
            }
        };

        struct InverseSquareCube {
            glm::vec3 m_center = glm::vec3(0.f);
            float m_scale = 1.0f;
            float m_eps = 0.f;

            InverseSquareCube(const glm::vec3& center = glm::vec3(0.f), const float scale = 1.0f, const float eps = 0.f) 
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
            glm::vec3 m_center = glm::vec3(0.f);
            glm::vec3 m_velocity = glm::vec3(0.f);
            float m_scale = 1.f;

            KineticBlob(const glm::vec3& center = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0), const float scale = 1.0f) 
                : m_center(center), m_velocity(velocity), m_scale(scale) {}

            float operator()(float x, float y, float z) const {
                return m_scale / (
                    (float) std::pow(m_center.x - x, 2) + 
                    (float) std::pow(m_center.y - y, 2) + 
                    (float) std::pow(m_center.z - z, 2)
                );
            }

            glm::vec3& update(const float dt) {
                m_center = m_center + m_velocity * dt;
                return m_center;
            }
        };

    }
}
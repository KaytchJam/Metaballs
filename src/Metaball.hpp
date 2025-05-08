#pragma once

#include <functional>
#include <vector>
#include <array>
// #include <ratio>

#include "utils/Vector3D.hpp"
#include "MarchingCubes.hpp"

#include "dependencies/glm/glm.hpp"
#include "dependencies/glfw-3.4/deps/glad/gl.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"

typedef std::function<float(
    const glm::vec3& center, 
    const glm::vec3& pt
)> MetaFunction;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

typedef struct control_point_t {
    glm::vec3 pos;
    float density;
    bool active;
} ctrl_pt_t;

struct Metaball {
    float sign;
    glm::vec3 position;
    MetaFunction func;
};

template<size_t GRID_SIZE>
class MetaballEngine {
private:
    Vector3D<ctrl_pt_t> ctrl_pts;
    std::vector<Metaball> metaballs;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    float isovalue = 1.f;

    static constexpr float CUBE_SIZE = 1.f / 10.f;

    static float blob_simple(const glm::vec3& center, const glm::vec3& pt) {
        return 1.f / (std::powf(center.x - pt.x, 2) + std::powf(center.y - pt.y, 2) + std::powf(center.z - pt.z, 2));
    }

    float compute_all(const glm::vec3& pt) const {
        float sum = 0.f;
        for (const Metaball& m : metaballs) {
            sum += m.sign * m.func(m.position, pt);
        }
        return sum;
    }

    // constructs a control point grid centered on the position
    // of Metaball 'm'
    Vector3D<ctrl_pt_t>& construct_control_grid(Vector3D<ctrl_pt_t>& ctrl_grid) {
        // assumption: we have a cube around each metaball
        // how big should the cube be? for now: 15 * 15 * 15

        constexpr size_t GSIZE_PLUS = GRID_SIZE + 1;
        const glm::vec3 grid_min = - glm::vec3(GRID_SIZE) * CUBE_SIZE * 0.5f;

        for (size_t z = 0; z < GSIZE_PLUS; z++) {
            for (size_t y = 0; y < GSIZE_PLUS; y++) {
                for (size_t x = 0; x < GSIZE_PLUS; x++) {

                    const glm::vec3 pos = grid_min + glm::vec3(x, y, z) * CUBE_SIZE;
                    const float density = this->compute_all(pos);

                    ctrl_grid.at(x,y,z) = ctrl_pt_t {
                        pos,
                        density,
                        density >= this->isovalue
                    };
                }
            }
        }

        return ctrl_grid;
    }

    static uint8_t compute_cube_index(const Vector3D<ctrl_pt_t>& ctrl_verts, const glm::ivec3& at) {
        glm::ivec3 offsets[8] = {
            {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
            {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
        };

        uint8_t index = 0;
        uint8_t iteration = 0;
        for (const glm::ivec3& offset : offsets) {
            const glm::ivec3 vertex_index = at + offset;
            const control_point_t& cp = ctrl_verts.at(vertex_index.x, vertex_index.y, vertex_index.z);

            if (cp.active) index |= (1 << iteration);  // i = 0 to 7
            iteration += 1;
        }

        return index;
    }

    static Vector3D<uint8_t>& get_cube_bits(const Vector3D<ctrl_pt_t>& ctrl_verts, Vector3D<uint8_t>& cube_bits) {
        const size_t SIZE = ctrl_verts.size_x() - 1;
        for (size_t layer = 0; layer < SIZE; layer++) {
            for (size_t row = 0; row < SIZE; row++) {
                for (size_t col = 0; col < SIZE; col++) {
                    cube_bits.at(col, row, layer) = MetaballEngine::compute_cube_index(ctrl_verts, glm::ivec3(col, row, layer));
                }
            }
        }

        return cube_bits;
    }

    static std::array<control_point_t, 8> get_cube_cpts(const Vector3D<ctrl_pt_t>& C, size_t i, size_t j, size_t k) {
        return std::array<control_point_t, 8> {
            C.at(i,j,k),
            C.at(i+1,j,k),
            C.at(i+1,j+1,k),
            C.at(i,j+1,k),
            C.at(i,j,k+1),
            C.at(i+1,j,k+1),
            C.at(i+1,j+1,k+1),
            C.at(i,j+1,k+1)
        };
    }

    static std::array<glm::vec3, 12> calc_edge_lerps(
        const std::array<control_point_t,8>& cube_cpts, 
        const float isovalue,
        uint8_t edge_mask
    ) {
        std::array<glm::vec3, 12> lerp_set = {};

        for (int edge = 0; edge < 12; edge++) {
            const int e1 = edge_mappings[edge][0];
            const int e2 = edge_mappings[edge][1];
    
            const glm::vec3& P1 = cube_cpts[e1].pos;
            const float V1 = cube_cpts[e1].density;
    
            const glm::vec3& P2 = cube_cpts[e2].pos;
            const float V2 = cube_cpts[e2].density;
    
            float denom = V2 - V1;
            if (std::abs(denom) < 1e-6f) denom = 1e-6f;
            lerp_set[edge] = P1 + (isovalue - V1) * (P2 - P1) / denom;
        }
    
        return lerp_set;
    }

    glm::vec3 gradient_at(const glm::vec3& center, const glm::vec3& p, const MetaFunction& f, float eps = 1e-3f) {
        return glm::normalize(glm::vec3(
            f(center, p + glm::vec3(eps, 0, 0)) - f(center, p - glm::vec3(eps, 0, 0)),
            f(center, p + glm::vec3(0, eps, 0)) - f(center, p - glm::vec3(0, eps, 0)),
            f(center, p + glm::vec3(0, 0, eps)) - f(center, p - glm::vec3(0, 0, eps))
        ));
    }

    glm::vec3 compute_gradient(const glm::vec3& p, const float eps = 1e-3f) const {
        const glm::vec3 dx = glm::vec3(eps, 0, 0);
        const glm::vec3 dy = glm::vec3(0, eps, 0);
        const glm::vec3 dz = glm::vec3(0, 0, eps);

        const glm::vec3 pdx = p + dx;
        const glm::vec3 mdx = p - dx;
        const glm::vec3 pdy = p + dy;
        const glm::vec3 mdy = p - dy;
        const glm::vec3 pdz = p + dz;
        const glm::vec3 mdz = p - dx;

        std::array<float, 6> sns = {}; // smash & splash
        for (const Metaball& m : metaballs) {
            sns[0] += m.sign * m.func(m.position, pdx);
            sns[1] += m.sign * m.func(m.position, mdx);
            sns[2] += m.sign * m.func(m.position, pdy);
            sns[3] += m.sign * m.func(m.position, mdy);
            sns[4] += m.sign * m.func(m.position, pdz);
            sns[5] += m.sign * m.func(m.position, mdz);
        }

        return glm::vec3(
            sns[0] - sns[1],
            sns[2] - sns[3],
            sns[4] - sns[5]
        );
    }

    glm::vec3 compute_normal(const glm::vec3& p, float eps = 1e-3f) {
        return -glm::normalize(compute_gradient(p));
    }

    MetaballEngine& build_mesh() {
        // since grid size is constant we'll just reuse the same buffer
        // Vector3D<uint8_t> cube_bits(GRID_SIZE);
        this->vertices.clear();
        this->indices.clear();

        //for (const Metaball& m : this->metaballs) {
        ctrl_pts = std::move(this->construct_control_grid(ctrl_pts));

        for (size_t k = 0; k < GRID_SIZE; k++) {
            for (size_t j = 0; j < GRID_SIZE; j++) {
                for (size_t i = 0; i < GRID_SIZE; i++) {
                    const uint8_t cube_index = MetaballEngine::compute_cube_index(ctrl_pts, glm::ivec3(i, j, k));

                    if (cube_index != 0x0 && cube_index != 0xFF) {
                        const std::array<control_point_t, 8> cube_cpts = MetaballEngine::get_cube_cpts(ctrl_pts, i, j, k);
                        const int cube_edge_mask = edge_masks[cube_index];
                        const std::array<glm::vec3, 12> lerp_points = MetaballEngine::calc_edge_lerps(
                            cube_cpts, this->isovalue, cube_edge_mask);
    
                        int edge_index = 0;
                        while (triTable[cube_index][edge_index] != -1 && edge_index + 2 < 16) {
    
                            const glm::vec3& p0 = lerp_points[triTable[cube_index][edge_index]];
                            const glm::vec3& p1 = lerp_points[triTable[cube_index][edge_index+1]];
                            const glm::vec3& p2 = lerp_points[triTable[cube_index][edge_index+2]];
    
                            // update vertices & normals
                            const uint32_t base_index = (uint32_t) this->vertices.size();
                            this->vertices.push_back(Vertex{ p0, compute_normal(p0)});
                            this->vertices.push_back(Vertex{ p1, compute_normal(p1)});
                            this->vertices.push_back(Vertex{ p2, compute_normal(p2)});
    
                            // update indices
                            this->indices.push_back(base_index);
                            this->indices.push_back(base_index + 1);
                            this->indices.push_back(base_index + 2);
    
                            edge_index += 3;
                        }
                    } 
                }
            }
        }
        //}

        return *this;
    }

public:
    MetaballEngine() : ctrl_pts(GRID_SIZE + 1) { }

    MetaballEngine& set_isovalue(float isoval = 1.0f) {
        this->isovalue = isoval;
        return *this;
    }

    size_t add_metaball(const glm::vec3& center, const MetaFunction& f = MetaballEngine::blob_simple) {
        size_t metaball_index = this->metaballs.size();
        this->metaballs.push_back(Metaball{1.f, center, std::move(f)});
        return metaball_index;
    }

    size_t subtract_metaball(const glm::vec3& center, const MetaFunction& f = MetaballEngine::blob_simple) {
        size_t metaball_index = this->metaballs.size();
        this->metaballs.push_back(Metaball{-1.f, center, std::move(f)});
        return metaball_index;
    }

    Metaball& get_metaball(size_t index) {
        assert(0 <= index && index < this->metaballs.size() && "INDEX IS OUT OF RANGE");
        return this->metaballs[index];
    }

    const std::vector<Vertex>& get_vertices() const {
        return this->vertices;
    }

    const std::vector<GLuint>& get_indices() const {
        return this->indices;
    }

    MetaballEngine& refresh() {
        this->build_mesh();
        return *this;
    }
};
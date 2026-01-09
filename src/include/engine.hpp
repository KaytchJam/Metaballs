#pragma once

// MBL
#include <isosurface.hpp>
#include <metaball.hpp>
#include <marcher.hpp>
#include <common/graphics.hpp>

// STD
#include <vector>
#include <array>

namespace mbl {
    typedef std::array<glm::vec3,12> LerpedEdgePoints; // Interpolated Edge Points
    typedef std::array<IsoPoint*,8> CubeOrderedIsopoints; // Isopoints reordered to match `edge_mappings`
    typedef std::array<common::graphics::Vertex, 16> OutVertices; // Cube vertices to be copied out
    typedef std::array<int32_t, 16> OutIndices; // Cube indices to be copied out

    /** Struct returned from MetaballEngine::compute_cube_bits. Returns the cube bit 'index' of a given
     * CubeView to be used to index into `edge_table` and `edge_mappings`. */
    struct CubeBitsResult {
        uint8_t cube_bits;
        const CubeOrderedIsopoints& cube_isopoints;
    };

    /** Data structure containing vertex, index, and count information for an individual
     * cube during Marching Cubes. */
    struct CubeTriData {
        const int32_t end_index;
        const OutVertices& vertices;
        const OutIndices& indices;
    };

    /** Engine for the construction of Metaballs */
    template <typename M = AggregateMetaball>
    class MetaballEngine {
        private:
            static_assert(std::is_base_of<DynamicMetaball, M>::value, "engine.hpp: MetaballEngine<M> -> M is not a type derived from DynamicMetaball.");

            IsoSurface field;
            std::vector<M> balls;
        
            float isovalue;
            bool is_dirty = true;

            int32_t num_valid_points = 0;
            common::graphics::MeshData mesh_data;

        public:
            /** Create a Metaball engine that constructs a `SCALAR FIELD` centered on `center` with a side length of `side_length`,
             * a resolution (# of divisions per axis in the scalar field), and an isovalue to test passed in metaballs against. */
            MetaballEngine(const glm::vec3& center, const float side_length, const int32_t resolution, const float isovalue = 1.0f);

            ~MetaballEngine() {}

            /** Add a metaball to this metaball engine. The index of the metaball is returned. */
            size_t add_metaball(M&& m) {
                size_t index = balls.size();
                balls.push_back(m);
                is_dirty = true;
                return index;
            }

            /** Get the metaball in this Metaball Engine at index i */
            M& get_metaball(size_t i) {
                return balls[i];
            }

            MetaballEngine<M>& make_dirty() {
                is_dirty = true;
                return *this;
            }

            /** Set the current isovalue of the Metaball engine to a different value */
            MetaballEngine<M>& set_isovalue(const float p_isovalue) { 
                this->is_dirty = dirty || (p_isovalue != isovalue);
                this->isovalue = p_isovalue;
                return *this; 
            }

            /** Returns the sum of all metaballs in the metaball engine
             * given x, y, and z coordinates. */
            float sum_metaballs(const float x, const float y, const float z) const;

            /** Returns the sum of all metaballs in the metaball engine
             * given a glm::vec3 position */
            float sum_metaballs(const glm::vec3& position) const;

            /** Compute gradient. */
            glm::vec3 compute_gradient(const glm::vec3& p, const float eps = 1e-3f) const;

            /** Compute normal. */
            glm::vec3 compute_normal(const glm::vec3& p, const float eps = 1e-3f) const;

            /** Computes and sets the density values for all IsoPoints in the field
             * this MetaballEngine spans over. */
            MetaballEngine& update_densities();

            /** Given a CubeView obtained by iterating through a Marching Cube Range, return the cube bits of
             * said cube where 0 means the cube bit is below the isovalue and 1 is equal to or above the
             * isovalue. Along with the cube bits a reference to the passed in CubeOrderedIsopoints array
             * is returned such that the bit ordering matches the IsoPoint ordering. */
            CubeBitsResult compute_cube_bits(CubeView& cube_view, CubeOrderedIsopoints& cube_isopoints);

            /** Interpolate cube edge points. */
            const LerpedEdgePoints& lerp_cube_edges(uint16_t cube_edge_bits, LerpedEdgePoints& cube_edge_points, const CubeOrderedIsopoints& cube_isopoints);

            /** Build cube tris. */
            CubeTriData build_cube_tris(const uint8_t cube_bits, const LerpedEdgePoints& leps, OutVertices& out_vertices, OutIndices& out_indices);

            /** Given an IsoField with set densities, constructs vertex
             * data from said field. */
            const common::graphics::MeshData& construct_mesh();
    };

    // MetaballEngine implementations
    
    #include "../MarchingCubes.hpp"

    static constexpr IndexDim cube_index_offsets[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    template <typename M>
    MetaballEngine<M>::MetaballEngine(const glm::vec3& center, const float side_length, const int32_t resolution, const float iso_value)
        : field(IsoSurface::construct(center, side_length / 2.f, resolution)), 
          balls(), 
          isovalue(iso_value),
          num_valid_points(0) {}

    template <typename M>
    float MetaballEngine<M>::sum_metaballs(const float x, const float y, const float z) const {
        float acc = 0.f;
        for (const M& ball : balls) {
            acc += ball(x, y, z);
        }
        return acc;
    }

    template <typename M>
    float MetaballEngine<M>::sum_metaballs(const glm::vec3& position) const {
        return sum_metaballs(position.x, position.y, position.z);
    }

    template <typename M>
    glm::vec3 MetaballEngine<M>::compute_gradient(const glm::vec3& p, const float eps) const {
        const glm::vec3 dx = glm::vec3(eps, 0, 0);
        const glm::vec3 dy = glm::vec3(0, eps, 0);
        const glm::vec3 dz = glm::vec3(0, 0, eps);

        const glm::vec3 pdx = p + dx;
        const glm::vec3 mdx = p - dx;
        const glm::vec3 pdy = p + dy;
        const glm::vec3 mdy = p - dy;
        const glm::vec3 pdz = p + dz;
        const glm::vec3 mdz = p - dx;

        std::array<float, 6> neighbors = {};
        for (const M& m : balls) {
            neighbors[0] += m.compute(pdx);
            neighbors[1] += m.compute(mdx);
            neighbors[2] += m.compute(pdy);
            neighbors[3] += m.compute(mdy);
            neighbors[4] += m.compute(pdz);
            neighbors[5] += m.compute(mdz);
        }

        return glm::vec3(
            neighbors[0] - neighbors[1],
            neighbors[2] - neighbors[3],
            neighbors[4] - neighbors[5]
        );
    }

    template <typename M>
    glm::vec3 MetaballEngine<M>::compute_normal(const glm::vec3& p, float eps) const {
        return -glm::normalize(compute_gradient(p));
    }

    template <typename M>
    MetaballEngine<M>& MetaballEngine<M>::update_densities() {
        num_valid_points = 0;
        for (IsoPoint& field_point : field.isopoints() ) {
            field_point.density = sum_metaballs(field_point.position);
            num_valid_points += (int32_t) (field_point.density >= isovalue);
        }
        return *this;
    }

    template <typename M>
    CubeBitsResult MetaballEngine<M>::compute_cube_bits(
        CubeView& cube_view, 
        CubeOrderedIsopoints& cube_isopoints
    ) {

        uint8_t cube_bits = 0;
        uint8_t mask = 0x1;
        uint8_t cube_isopoint_index = 0;

        // Set each bit in cube_bit based on whether its corresponding isopoint satisfies the isovalue threshold
        for (const IndexDim& offset : cube_index_offsets) {
            IsoPoint& cube_point = cube_view.at(offset.x, offset.y, offset.z);
            cube_bits = cube_bits | (mask * (uint8_t) (cube_point.density >= isovalue));
            mask = mask << 1;

            cube_isopoints[cube_isopoint_index] = &cube_point;
            cube_isopoint_index += 1;
        }

        return CubeBitsResult {
            cube_bits,
            cube_isopoints
        };
    }

    template <typename M>
    const LerpedEdgePoints& MetaballEngine<M>::lerp_cube_edges(
        uint16_t cube_edge_bits, 
        LerpedEdgePoints& cube_edge_points,
        const CubeOrderedIsopoints& cube_isopoints
    ) {
        uint8_t cube_edge_index = 0;
        while (cube_edge_bits != 0) {
            if ((0x1 & cube_edge_bits) == 1) {
                const int (&edge)[2] = edge_mappings[cube_edge_index];
                const IsoPoint& I1 = *cube_isopoints[edge[0]];
                const IsoPoint& I2 = *cube_isopoints[edge[1]];

                float denominator = I2.density - I1.density;
                cube_edge_points[cube_edge_index] = I1.position + (isovalue - I1.density) * (I2.position - I1.position) / denominator;
            }

            cube_edge_bits = cube_edge_bits >> 1;
            cube_edge_index += 1;
        }

        return cube_edge_points;
    }

    template <typename M>
    CubeTriData MetaballEngine<M>::build_cube_tris(
        const uint8_t cube_bits, 
        const LerpedEdgePoints& leps, 
        OutVertices& out_vertices, 
        OutIndices& out_indices
    ) {
        const int32_t (&edge_ordering)[16] = triTable[cube_bits];
        int32_t eoi = 0;

        while (edge_ordering[eoi] != -1 && eoi < 16) {
            // Setting Vertex Data
            out_vertices[eoi].position = leps[edge_ordering[eoi]];
            out_vertices[eoi].normal = compute_normal(out_vertices[eoi].position); 

            out_vertices[eoi + 1].position = leps[edge_ordering[eoi+1]];
            out_vertices[eoi + 1].normal = compute_normal(out_vertices[eoi+1].position); 

            out_vertices[eoi + 2].position = leps[edge_ordering[eoi+2]];
            out_vertices[eoi + 2].normal = compute_normal(out_vertices[eoi+2].position); 

            // Setting Index Data
            int32_t index_at = eoi + (int32_t) mesh_data.indices.size();
            out_indices[eoi] = index_at;
            out_indices[eoi + 1] = index_at + 1;
            out_indices[eoi + 2] = index_at + 2;

            eoi += 3;
        }

        return CubeTriData {
            eoi,
            out_vertices,
            out_indices
        };
    }

    template <typename M>
    const common::graphics::MeshData& MetaballEngine<M>::construct_mesh() {
        if (!is_dirty) {
            return mesh_data;
        }

        is_dirty = false;
        const int32_t valid_points = update_densities().num_valid_points;
        
        mesh_data.vertices.clear();
        mesh_data.vertices.reserve(valid_points);
        mesh_data.indices.clear();
        mesh_data.indices.reserve(valid_points);

        // Buffers we'll reuse multiple times in this loop
        CubeOrderedIsopoints ordered_iso_points = {};
        LerpedEdgePoints lerped_edge_points = {};
        OutVertices cube_out_vertices = {};
        OutIndices cube_out_indices = {};
        
        for (CubeView cv : MarchingCubeRange(field)) {
            const CubeBitsResult cbr = compute_cube_bits(cv, ordered_iso_points);

            if (cbr.cube_bits != 0x0 && cbr.cube_bits != 0xFF) {
                const int16_t cube_edge_bits = edge_table[cbr.cube_bits];
                const LerpedEdgePoints& leps = lerp_cube_edges(cube_edge_bits, lerped_edge_points, cbr.cube_isopoints);
                const CubeTriData tri_data = build_cube_tris(cbr.cube_bits, leps, cube_out_vertices, cube_out_indices);

                std::copy(tri_data.vertices.begin(), tri_data.vertices.begin() + tri_data.end_index, std::back_inserter(mesh_data.vertices));
                std::copy(tri_data.indices.begin(), tri_data.indices.begin() + tri_data.end_index, std::back_inserter(mesh_data.indices));
            }
        }

        return mesh_data;
    }
}
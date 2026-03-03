#pragma once

#include <fieldrange.hpp>
#include <vector>
#include <iostream>

namespace mbl {
    /** Represents a cube corner point in an IsoSurface. */
    struct IsoPointProxy {
        lalg::vec3& position;
        float& density;
    };

    /** Representation of a cube corner point in an IsoSurface */
    struct IsoPoint {
        lalg::vec3 position = lalg::vec3(0.f);
        float density = 0.f;

        IsoPoint() {}
        IsoPoint(const lalg::vec3& pos, const float dens) : position(pos), density(dens) {}
    };

    typedef lalg::ivec3 IndexDim;

    /** Struct for mapping between linear indices & 3d indices
     * indices. Takes in an `IndexDim` "dim" as a context. */
    struct IndexCompactor {
        const IndexDim dim;

        IndexCompactor(const IndexDim& dim) : dim(dim) {}
        IndexCompactor(int32_t x, int32_t y, int32_t z) : dim(IndexDim(x,y,z)) {}
        IndexCompactor(int32_t x) : dim(IndexDim(x,x,x)) {}

        int32_t flatten(int32_t x, int32_t y, int32_t z) const;
        int32_t flatten_at(int32_t value, int32_t axis) const;
        IndexDim unflatten(int32_t i) const;

        const IndexDim& dimensions() const;
    };
    
    /** Cube shaped IsoSurface with side lengths 'length' is created, centered
     * on position 'center'. */
    class IsoSurface {
    private:
        float m_side_length;
        uint32_t m_partitions;
        lalg::vec3 m_center_position;
        std::vector<IsoPoint> m_isopoints;

        IsoSurface(const lalg::vec3& center, float length, uint32_t partitions);
    public:
        ~IsoSurface() = default;

        std::vector<IsoPoint>& isopoints();
        const std::vector<IsoPoint>& isopoints() const;
        
        /** Get the IsoPoint on the IsoSurface at index 'i' */
        IsoPoint& get(uint32_t i);
        const IsoPoint& get(uint32_t) const;

        IsoPoint& get(uint32_t i, uint32_t j, uint32_t k);
        const IsoPoint& get(uint32_t i, uint32_t j, uint32_t k) const;
        
        lalg::vec3& get_position(uint32_t i);
        const lalg::vec3& get_position(uint32_t i) const;

        float& get_density(uint32_t i);
        const float& get_density(uint32_t i) const;
        
        /** Initializes an IsoSurface object centered at position 'center', with
         * side lengths 'side_length', and 'partitions' partitions. Partitions
         * must be an EVEN positive integer.
         * 
         * If partitions is odd, `new_partitions = partitions - 1` */
        static IsoSurface construct(
            const lalg::vec3& center,
            const float side_length,
            uint32_t partitions
        );

        IsoPoint* data();
        const IsoPoint* data() const;
        
        /** Returns an IndexCompactor that uses
         * the shape of the IsoSurface as a context */
        IndexCompactor compactor() const;

        /** Returns the number of indices (or points) within the IsoSurface */
        size_t indices() const;

        const lalg::vec3& get_center() const {
            return this->m_center_position;
        }
        
        /** Returns the shape (indices per axis) of this IsoSurface 
         * as an IndexDim */
        IndexDim shape() const;

        /** Given a position, maps said point to a floating-point index  */
        lalg::vec3 position_to_index(const lalg::vec3& position) {
            const mbl::lalg::ivec3 m = shape() / 2;
            return (position - m_center_position) * (m.x  / m_side_length) + m;
        }

        /** Returns the side length of this IsoSurface */
        float length() const;
        
        /** For debugging */
        void _print_positions() const {
            const IndexDim surface_shape = this->shape();
            for (const IsoPoint& isopoint : this->isopoints()) {
                std::cout << "[" 
                    << isopoint.position.x << "," 
                    << isopoint.position.y << "," 
                    << isopoint.position.z << 
                "]" << std::endl;
            }
        }
    };
}
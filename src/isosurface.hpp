#pragma once

#include "dependencies/glm/glm.hpp"
#include "fieldrange.hpp"
#include <vector>
#include <iostream>

namespace mball {
    /** Represents a cube corner point in an IsoSurface. */
    struct IsoPoint {
        glm::vec3 position;
        float density;
    };

    typedef glm::ivec3 IndexDim;

    /** Struct for mapping between linear indices & 3d indices
     * indices. Takes in an `IndexDim` "dim" as a context. */
    struct IndexCompactor {
        const IndexDim dim;

        IndexCompactor(const IndexDim& dim) : dim(dim) {}
        IndexCompactor(int x, int y, int z) : dim(IndexDim(x,y,z)) {}
        IndexCompactor(int x) : dim(IndexDim(x,x,x)) {}

        int flatten(int x, int y, int z) const;
        int flatten_at(int value, int axis) const;
        IndexDim unflatten(int i) const;
        const IndexDim& dimensions() const;
    };

    /** Cube shaped IsoSurface with side lengths 'length' is created, centered
     * on position 'center'. */
    class IsoSurface {
    private:
        float m_side_length;
        uint32_t m_partitions;
        glm::vec3 m_center_position;
        std::vector<IsoPoint> m_isopoints;

        IsoSurface(const glm::vec3& center, float length, uint32_t partitions);
    public:
        ~IsoSurface() = default;
    
        using iterator = std::vector<IsoPoint>::iterator;
        using const_iterator = std::vector<IsoPoint>::const_iterator;

        IsoSurface::iterator begin();
        IsoSurface::iterator end();
        IsoSurface::const_iterator begin() const;
        IsoSurface::const_iterator end() const;

        /** Get the IsoPoint on the IsoSurface at index 'i' */
        IsoPoint& get(int i);
        const IsoPoint& get(int i) const;

        /** Initializes an IsoSurface object centered at position 'center', with
         * side lengths 'side_length', and 'partitions' partitions. Partitions
         * must be an EVEN positive integer.
         * 
         * If partitions is odd, `new_partitions = partitions - 1` */
        static IsoSurface construct(const glm::vec3& center, const float side_length, uint32_t partitions);

        /** Returns an IndexCompactor that uses
         * the shape of the IsoSurface as a context */
        IndexCompactor compactor() const;

        /** Returns the number of indices (or points) within the IsoSurface */
        size_t indices() const;

        /** Returns the shape (indices per axis) of this IsoSurface 
         * as an IndexDim */
        IndexDim shape() const;

        /** Returns the side length of this IsoSurface */
        float length() const;

        /** For debugging */
        void _print() const {
            const IndexDim surface_shape = this->shape();
            for (const IsoPoint& iso : *this) {
                const glm::vec3& pos = iso.position;
                std::cout << "[" << pos.x << "," << pos.y << "," << pos.z << "]" << std::endl;
            }
        }
    };
}
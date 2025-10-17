#pragma once

#include "dependencies/glm/glm.hpp"
#include "fieldrange.hpp"
#include <vector>
#include <iostream>

static unsigned int my_pow_int(const unsigned int base, const unsigned int exp) {
    if (base == 0 || base == 1) return base;
    unsigned int acc = 1;
    for (int j = 0; j < exp; j++) { acc *= base; }
    return acc;
}

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
        float side_length;
        unsigned int partitions;
        glm::vec3 center_position;
        std::vector<IsoPoint> isopoints;

        IsoSurface(const glm::vec3& center, float length, unsigned int partitions) 
            : center_position(center), side_length(length), 
            partitions(partitions), isopoints() {
            isopoints.reserve(my_pow_int(partitions + 1, 3));
        }
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
        static IsoSurface construct(const glm::vec3& center, float side_length, unsigned int partitions) {
            assert(partitions > 1);
            
            partitions -= (partitions & 0x1) == 1; // makes partition odd if even
            const int axis_indices = partitions + 1;
            const IndexDim mid_idx = IndexDim(axis_indices) / 2;

            IsoSurface surface = IsoSurface(center, side_length, partitions);

            glm::vec3 offset;
            glm::vec3 ratios;
            const float half_indices = (float) mid_idx.x;
            for (const IndexDim& p_idx : FieldRange(0, axis_indices)) {
                offset = glm::vec3(p_idx - mid_idx);
                ratios = offset / half_indices;
                surface.isopoints.push_back( IsoPoint{ ratios * side_length, 0.0 } );
            }

            return surface;
        }

        /** Returns an IndexCompactor that uses
         * the shape of the IsoSurface as a context */
        IndexCompactor compactor() const {
            return IndexCompactor(partitions + 1);
        }

        /** Returns the number of indices (or points) within the IsoSurface */
        size_t indices() const {
            return isopoints.size();
        }

        /** Returns the shape (indices per axis) of this IsoSurface 
         * as an IndexDim */
        IndexDim shape() const {
            return IndexDim(partitions + 1);
        }

        /** Returns the side length of this IsoSurface */
        float length() const {
            return this->side_length;
        }

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
#pragma once

#include "../dependencies/glm/glm.hpp"

#include <fieldrange.hpp>
#include <vector>
#include <iostream>

namespace mball {
    /** Represents a cube corner point in an IsoSurface. */
    struct IsoPointProxy {
        glm::vec3& position;
        float& density;
    };

    typedef glm::ivec3 IndexDim;

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

    /** Give access to ranging over a vector type without explicit
     * access to the inner vector */
    template <typename ValueType>
    struct ProtectedVectorRange {
    private:
        std::vector<ValueType>& V;
    public:
        using iterator = typename std::vector<ValueType>::iterator;
        using const_iterator = typename std::vector<ValueType>::const_iterator;

        ProtectedVectorRange(std::vector<ValueType>& V) : V(V) {}

        iterator begin() { return V.begin(); }
        const_iterator begin() const { return V.begin(); }
        iterator end() { return V.end(); }
        const_iterator end() const { return V.end(); }
    };

    template <typename ValueType>
    struct ConstProtectedVectorRange {
    private:
        const std::vector<ValueType>& V;
    public:
        using iterator = typename std::vector<ValueType>::iterator;
        using const_iterator = typename std::vector<ValueType>::const_iterator;

        ConstProtectedVectorRange(const std::vector<ValueType>& V) : V(V) {}

        const_iterator begin() const { return V.begin(); }
        const_iterator end() const { return V.end(); }
    };

    struct IsoPointProxies;
    
    /** Cube shaped IsoSurface with side lengths 'length' is created, centered
     * on position 'center'. */
    class IsoSurface {
    private:
        float m_side_length;
        uint32_t m_partitions;
        glm::vec3 m_center_position;

        std::vector<glm::vec3> m_positions;
        std::vector<float> m_densities;

        IsoSurface(const glm::vec3& center, float length, uint32_t partitions);
    public:
        ~IsoSurface() = default;

        using position_iterator = std::vector<glm::vec3>::iterator;
        using density_iterator = std::vector<float>::iterator;
        
        ProtectedVectorRange<glm::vec3> positions();
        ConstProtectedVectorRange<glm::vec3> positions() const;

        ProtectedVectorRange<float> densities();
        ConstProtectedVectorRange<float> densities() const;

        IsoPointProxies isopoints();
        
        /** Get the IsoPoint on the IsoSurface at index 'i' */
        IsoPointProxy get(uint32_t i);
        
        glm::vec3& get_position(uint32_t i);
        const glm::vec3& get_position(uint32_t i) const;

        float& get_density(uint32_t i);
        const float& get_density(uint32_t i) const;

        /** Initializes an IsoSurface object centered at position 'center', with
         * side lengths 'side_length', and 'partitions' partitions. Partitions
         * must be an EVEN positive integer.
         * 
         * If partitions is odd, `new_partitions = partitions - 1` */
        static IsoSurface construct(
            const glm::vec3& center, 
            const float side_length, 
            uint32_t partitions
        );
        
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
        void _print_positions() const {
            const IndexDim surface_shape = this->shape();
            for (const glm::vec3& pos : positions()) {
                std::cout << "[" << pos.x << "," << pos.y << "," << pos.z << "]" << std::endl;
            }
        }
    };

    struct IsoPointProxies {
        IsoSurface& m_parent;

        IsoPointProxies(IsoSurface& surface) : m_parent(surface) {}

        using IterPosition = IsoSurface::position_iterator;
        using IterDensity = IsoSurface::density_iterator;

        struct IPPIterator {
        private:
            IterPosition m_pos_iter;
            IterDensity m_density_iter;
        public:
    
            using value_type = IsoPointProxy;
            using reference_type = value_type;
            using pointer_type = void;
    
            IPPIterator(IterPosition iterP, IterDensity iterD)
                : m_pos_iter(iterP), m_density_iter(iterD) {}

            IsoPointProxy operator*() {
                return IsoPointProxy{ *m_pos_iter, *m_density_iter };
            }

            IPPIterator& operator++() {
                ++m_pos_iter;
                ++m_density_iter;
                return *this;
            }

            IPPIterator operator++(int) {
                IPPIterator prev = *this;
                ++(*this);
                return prev;
            }

            bool operator==(const IPPIterator& other) const {
                return m_pos_iter == other.m_pos_iter && m_density_iter == other.m_density_iter;
            }

            bool operator!=(const IPPIterator& other) const {
                return !(*this == other);
            }
        };

        using iterator = IPPIterator;

        iterator begin() {
            return iterator(
                m_parent.positions().begin(), 
                m_parent.densities().begin()
            );
        }

        iterator end() {
            return iterator(
                m_parent.positions().end(), 
                m_parent.densities().end()
            );
        }
    };
}
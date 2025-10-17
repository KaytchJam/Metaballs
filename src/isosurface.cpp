
#include <isosurface.hpp>

using namespace mball;

// INDEX COMPACTOR FUNCTIONS

int IndexCompactor::flatten(int x, int y, int z) const {
    return x + y * dim.x + z * dim.y * dim.x;
}

int IndexCompactor::flatten_at(int value, int axis) const {
    switch (axis) {
        case 0:
            return value;
        case 1:
            return value * dim.x;
        case 2:
            return value * dim.x * dim.y;
    }

    return 0;
}

IndexDim IndexCompactor::unflatten(int i) const {
    return IndexDim(
        i % dim.x, 
        i / dim.x,
        i / (dim.x * dim.y)
    );
}

const IndexDim& IndexCompactor::dimensions() const {
    return dim;
}

// ISOSURFACE FUNCTIONS & HELPERS

uint32_t pow_int(uint32_t base, uint32_t exp) {
    if (base == 0 || base == 1) return base;
    uint32_t acc = 1;
    for (int j = 0; j < exp; j++) { acc *= base; }
    return acc;
}

IsoSurface::IsoSurface(
    const glm::vec3& center, 
    const float side_length,
    const uint32_t partitions
) : m_center_position(center), m_side_length(side_length), 
    m_partitions(partitions), m_isopoints() 
{
    m_isopoints.reserve(pow_int(partitions + 1, 3));
}

IsoSurface IsoSurface::construct(const glm::vec3& center, const float side_length, uint32_t partitions) {
    assert(partitions > 1);
    
    partitions -= (partitions & 0x1) == 1; // makes partition even if odd
    const int axis_indices = partitions + 1;
    const IndexDim mid_idx = IndexDim(axis_indices) / 2;

    IsoSurface surface = IsoSurface(center, side_length, partitions);

    glm::vec3 offset;
    glm::vec3 ratios;
    const float half_indices = (float) mid_idx.x;
    for (const IndexDim& p_idx : FieldRange(0, axis_indices)) {
        offset = glm::vec3(p_idx - mid_idx);
        ratios = offset / half_indices;
        surface.m_isopoints.push_back( IsoPoint{ ratios * side_length, 0.0 } );
    }

    return surface;
}

IndexCompactor IsoSurface::compactor() const {
    return IndexCompactor(m_partitions + 1);
}

size_t IsoSurface::indices() const {
    return m_isopoints.size();
}

IndexDim IsoSurface::shape() const {
    return IndexDim(m_partitions + 1);
}

float IsoSurface::length() const {
    return this->m_side_length;
}

IsoSurface::iterator IsoSurface::begin() {
    return m_isopoints.begin();
}

IsoSurface::const_iterator IsoSurface::begin() const {
    return m_isopoints.begin();
}

IsoSurface::iterator IsoSurface::end() {
    return m_isopoints.end();
}

IsoSurface::const_iterator IsoSurface::end() const {
    return m_isopoints.end();
}

IsoPoint& IsoSurface::get(int i) {
    return m_isopoints[i];
}
const IsoPoint& IsoSurface::get(int i) const {
    return m_isopoints[i];
}
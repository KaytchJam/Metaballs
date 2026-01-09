#pragma once

#include "../dependencies/glm/glm.hpp"

#include <intrange.hpp>
#include <array>
#include <iterator>

typedef glm::ivec3 IndexDim;

/** Represents a range across different indices. */
struct FieldRange {
    std::array<IntRange,3> m_bounds;

    FieldRange(const std::array<IntRange,3>& bounds);
    FieldRange(const std::array<int32_t,6>& bounds);
    FieldRange(int32_t low, int32_t high);
    FieldRange(const IntRange& ir);
    // FieldRange(const IndexDim& highs);
    ~FieldRange() = default;
    
    struct FieldRangeIterator {
        IndexDim at;
        const std::array<IntRange,3>& m_bounds;

        FieldRangeIterator(const std::array<IntRange,3>& b);
        FieldRangeIterator(const std::array<IntRange,3>& b, const IndexDim& at);
        ~FieldRangeIterator() = default;
        
        using value_type = IndexDim;
        using reference = const IndexDim&;
        using pointer = void;
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        
        reference operator*();
        FieldRangeIterator& operator++();
        FieldRangeIterator operator++(int);

        bool operator==(const FieldRangeIterator& other) const;
        bool operator!=(const FieldRangeIterator& other) const;
    };
    
    typedef FieldRangeIterator iterator;

    FieldRange::iterator begin() const;
    FieldRange::iterator end() const;

    IndexDim low() const;
    IndexDim high() const;
};
#include "fieldrange.hpp"

// HELPER FUNCTIONS

template <size_t D>
static std::array<IntRange,D / 2> arr_to_range(const std::array<int,D>& b) {
    std::array<IntRange, D/2> int_ranges;
    
    int range_idx = 0;
    for (int i = 0; i < D; i += 2) {
        int_ranges[range_idx] = IntRange(b[i], b[i+1]);
        range_idx += 1;
    }
    
    return int_ranges;
}

template <size_t S>
static std::array<IntRange, S> duplicate_range(const IntRange& ir) {
    std::array<IntRange, S> int_ranges;
    for (IntRange& r : int_ranges) {
        r = ir;
    }
    return int_ranges;
}

// HELPER ALIASES

template <size_t D>
using IRangeArr = std::array<IntRange,D>;
using FRIter = FieldRange::iterator;

// FIELD RANGE DEFINITIONS

FieldRange::FieldRange(const std::array<IntRange,3>& p_bounds) : bounds(p_bounds) {}
FieldRange::FieldRange(const std::array<int,6>& p_bounds) : bounds(arr_to_range<6>(p_bounds)) {}
FieldRange::FieldRange(int p_low, int p_high) : bounds(duplicate_range<3>(IntRange(p_low, p_high))) {}
FieldRange::FieldRange(const IntRange& p_ir) : bounds(duplicate_range<3>(p_ir)) {}

FRIter FieldRange::begin() const { return FieldRangeIterator(bounds); }
FRIter FieldRange::end() const { return FieldRangeIterator(bounds, IndexDim(0,0,bounds[2].high())); }

IndexDim FieldRange::low() const {
    return IndexDim(bounds[0].low(), bounds[1].low(), bounds[2].low());
}

IndexDim FieldRange::high() const {
    return IndexDim(bounds[0].high(), bounds[1].high(), bounds[2].high());
}

// FIELD RANGE ITERATOR DEFINITIONS

FRIter::FieldRangeIterator(const std::array<IntRange,3>& p_bounds)
    : at(p_bounds[0].low(), p_bounds[1].low(), p_bounds[2].low()), 
    bounds(p_bounds) {}

FRIter::FieldRangeIterator(
    const IRangeArr<3>& p_bounds, 
    const IndexDim& p_at
) : at(p_at), bounds(p_bounds) {}

FRIter::reference_type FieldRange::FieldRangeIterator::operator*() {
    return at;
}

FRIter& FieldRange::FieldRangeIterator::operator++() {
    at.x += 1;
    if (at.x >= bounds[0].high()) {
        at.x = bounds[0].low();
        at.y++;
        if (at.y >= bounds[1].high()) {
            at.y = bounds[1].low();
            at.z++;
        }
    }

    return *this;
}

FRIter FieldRange::FieldRangeIterator::operator++(int) {
    FieldRangeIterator prev = FieldRangeIterator(bounds, at);
    ++(*this);
    return prev;
}
        
bool FieldRange::FieldRangeIterator::operator==(const FieldRangeIterator& other) const {
    return bounds == other.bounds && at == other.at;
}

bool FieldRange::FieldRangeIterator::operator!=(const FieldRangeIterator& other) const {
    return !(*this == other);
}
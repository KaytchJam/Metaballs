
#include <fieldrange.hpp>

// HELPER ALIASES

template <size_t D>
using IRangeArr = std::array<IntRange,D>;
using FRIter = FieldRange::iterator;

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

static IndexDim project_range_to_dim(const IRangeArr<3>& bounds, int (IntRange::* getter)() const) {
    return IndexDim((bounds[0].*getter)(), (bounds[1].*getter)(), (bounds[2].*getter)());
}

// FIELD RANGE DEFINITIONS

FieldRange::FieldRange(const IRangeArr<3>& p_bounds) : m_bounds(p_bounds) {}
FieldRange::FieldRange(const std::array<int,6>& p_bounds) : m_bounds(arr_to_range<6>(p_bounds)) {}
FieldRange::FieldRange(int p_low, int p_high) : m_bounds(duplicate_range<3>(IntRange(p_low, p_high))) {}
FieldRange::FieldRange(const IntRange& p_ir) : m_bounds(duplicate_range<3>(p_ir)) {}

FRIter FieldRange::begin() const { return FieldRangeIterator(m_bounds); }
FRIter FieldRange::end() const { return FieldRangeIterator(m_bounds, IndexDim(0,0, m_bounds[2].high())); }

IndexDim FieldRange::low() const {
    return project_range_to_dim(m_bounds, IntRange::low);
}

IndexDim FieldRange::high() const {
    return project_range_to_dim(m_bounds, IntRange::high);
}

// FIELD RANGE ITERATOR DEFINITIONS

FRIter::FieldRangeIterator(const IRangeArr<3>& p_bounds)
    : at(project_range_to_dim(p_bounds, &IntRange::low)), 
    m_bounds(p_bounds) {}

FRIter::FieldRangeIterator(
    const IRangeArr<3>& p_bounds, 
    const IndexDim& p_at
) : at(p_at), m_bounds(p_bounds) {}

FRIter::reference FieldRange::FieldRangeIterator::operator*() {
    return at;
}

FRIter& FieldRange::FieldRangeIterator::operator++() {
    at.x += 1;
    if (at.x >= m_bounds[0].high()) {
        at.x = m_bounds[0].low();
        at.y++;
        if (at.y >= m_bounds[1].high()) {
            at.y = m_bounds[1].low();
            at.z++;
        }
    }

    return *this;
}

FRIter FieldRange::FieldRangeIterator::operator++(int) {
    FieldRangeIterator prev = FieldRangeIterator(m_bounds, at);
    ++(*this);
    return prev;
}
        
bool FieldRange::FieldRangeIterator::operator==(const FieldRangeIterator& other) const {
    return m_bounds == other.m_bounds && at == other.at;
}

bool FieldRange::FieldRangeIterator::operator!=(const FieldRangeIterator& other) const {
    return !(*this == other);
}
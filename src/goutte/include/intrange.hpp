#pragma once

#include <iterator>
#include <cstdint>

/** Range representing the interval [low, high). Note that high is not included in the range */
class IntRange {
public:
    int32_t m_low;
    int32_t m_high;

    IntRange();
    IntRange(int32_t high);
    IntRange(int32_t low, int32_t high);
    IntRange(const IntRange& ir);
    ~IntRange() = default;

    int32_t low() const;
    int32_t high() const;
    uint32_t size() const;

    struct IntRangeIterator {
        // Iterator Traits
        using value_type = int32_t;
        using reference = value_type; 
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        int32_t m_at;
        const int32_t m_high;
        IntRangeIterator(int32_t start, int32_t high);

        value_type operator*() const;
        IntRangeIterator& operator++();
        IntRangeIterator operator++(int);

        bool operator==(const IntRangeIterator& other) const;
        bool operator!=(const IntRangeIterator& other) const;
    };

    typedef IntRangeIterator iterator;

    iterator begin() const;
    iterator end() const;
    
    bool contains(int i) const;
    bool operator==(const IntRange& other)  const;
};
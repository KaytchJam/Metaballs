#pragma once
#include <iterator>

/** Range representing the interval [low, high). Note that high is not included in the range */
class IntRange {
    private:
        int m_low;
        int m_high;
    public:
        IntRange();
        IntRange(int high);
        IntRange(int low, int high);
        IntRange(const IntRange& ir);
        ~IntRange();

        int low() const;
        int high() const;

        struct IntRangeIterator {
            // Iterator Traits
            using value_type = int;
            using difference_type = int;
            using pointer_type = void;
            using reference_type = int; 
            using iterator_category = std::input_iterator_tag;

            int m_at;
            int m_high;
            IntRangeIterator(int start, int high);

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
#include "intrange.hpp"

// IntRange definitions

IntRange::IntRange() : m_low(0), m_high(0) {}
IntRange::IntRange(int high) : m_low(0), m_high(high) {}
IntRange::IntRange(int low, int high) : m_low(low), m_high(high) {}
IntRange::IntRange(const IntRange& ir) : m_low(ir.m_low), m_high(ir.m_high) {}
IntRange::~IntRange() {}

int IntRange::low() const { return this->m_low; }
int IntRange::high() const { return this->m_high; }
IntRange::iterator IntRange::begin() const { return IntRange::iterator(this->m_low, this->m_high); }
IntRange::iterator IntRange::end() const { return IntRange::iterator(this->m_high, this->m_high); }
bool IntRange::contains(int i) const { return this->m_low <= i && i < this->m_high; }

bool IntRange::operator==(const IntRange& other) const {
    return m_low == other.m_low && m_high == other.m_high;
}

// IntRangeIterator defintions

IntRange::iterator::IntRangeIterator(int start, int high) : m_at(start), m_high(high) {}

IntRange::iterator::value_type IntRange::iterator::operator*() const {
    return this->m_at;
}

IntRange::iterator& IntRange::iterator::operator++() {
    this->m_at = std::min(this->m_at + 1, this->m_high);
    return *this;
}

IntRange::iterator IntRange::iterator::operator++(int) {
    IntRange::iterator prev = IntRange::iterator(this->m_at, this->m_high);
    ++(*this);
    return prev;
}

bool IntRange::iterator::operator==(const IntRange::iterator& other) const {
    return this->m_at == other.m_at && this->m_high == other.m_high;
}

bool IntRange::iterator::operator!=(const IntRange::iterator& other) const {
    return !(*this == other);
}
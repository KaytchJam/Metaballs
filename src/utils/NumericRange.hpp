#pragma once
#include <iostream>

struct numeric_range {
	size_t m_start;
	size_t m_end; // exclusive

	size_t min() const {
		return this->m_start;
	}

	size_t max() const {
		return this->m_end;
	}

	bool in_range(size_t s) const {
		return s > this->m_start && s < this->m_end;
	}

	// Returns the size of the range, m_end - m_start
	size_t size() const {
		return m_end - m_start;
	}

	numeric_range(size_t end) : m_start(0), m_end(end) {}
	numeric_range(size_t start, size_t end) : m_start(start), m_end(end) {}

	struct numeric_range_iterator {
	private:
		size_t m_at = 0;
		size_t m_end = 0;
	public:
		using Category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using Pointer = size_t;
		using value_type = size_t;
		using Reference = size_t; // fake reference, we just copy the value

		numeric_range_iterator() {}
		numeric_range_iterator(size_t start, size_t end) : m_at(start), m_end(end) {}

		Reference operator*() const {
			return this->m_at;
		}

		value_type operator->() const {
			return this->m_at;
		}

		numeric_range_iterator& operator++() {
			++(this->m_at);
			return *this;
		}

		numeric_range_iterator operator++(int) {
			numeric_range_iterator temp = *this;
			++(*this);
			return temp;
		}

		numeric_range_iterator& operator--() {
			--(this->m_at);
			return *this;
		}

		numeric_range_iterator operator--(int) {
			numeric_range_iterator temp = *this;
			--(*this);
			return temp;
		}

		friend bool operator<(const numeric_range_iterator& it1, const numeric_range_iterator& it2) {
			return it1.m_at < it2.m_at;
		}

		friend bool operator==(const numeric_range_iterator& nrit1, const numeric_range_iterator& nrit2) {
			return nrit1.m_at == nrit2.m_at;
		}

		friend bool operator!=(const numeric_range_iterator& nrit1, const numeric_range_iterator& nrit2) {
			return nrit1.m_at != nrit2.m_at;
		}
	};

	typedef numeric_range_iterator iterator;

	iterator begin() const {
		return numeric_range_iterator(this->m_start, this->m_end);
	}

	iterator end() const {
		return numeric_range_iterator(this->m_end, this->m_end);
	}

	friend std::ostream& operator<<(std::ostream& os, const numeric_range& nr) {
		return os << "(" << nr.min() << ", " << nr.max() << ")";
	}
};
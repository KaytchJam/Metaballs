#include <dsa/unionfind.hpp>

using namespace gtt::dsa;

// UNION FIND

UnionFind::UnionFind(size_t n) : parents(n), sizes(n,1), stack() {
    for (uf_index_t i = 0; i < (uf_index_t) n; i++) {
        parents[i] = i;
    }
    stack.reserve(n);
}

UnionFind::~UnionFind() {}

UnionFind::uf_index_t UnionFind::find_mut(const uf_index_t x) {
    uf_index_t top_parent = parents[x];
    stack.push(StackFrame(false, x));
    while (!stack.empty()) {
        StackFrame& frame = stack.top();

        // backtracking
        if (*frame) {
            parents[frame.index] = top_parent;
            stack.pop();

        // first visit
        } else {
            frame.item = true;
            if (parents[frame.index] == frame.index) {
                top_parent = frame.index;
                stack.pop();
            }
        }
    }

    stack.reset();
    return top_parent;
}

UnionFind::uf_index_t UnionFind::find(uf_index_t x) const {
    uf_index_t parent = parents[x];
    while (parent != x) {
        x = parent;
        parent = parents[x];
    }
    return parent;
}

bool UnionFind::is_root(const uf_index_t x) const {
    return x == parents[x];
}

UnionFind& UnionFind::unite(uf_index_t a, uf_index_t b) {
    a = find_mut(a);
    b = find_mut(b);

    if (a != b) {
        if (sizes[a] < sizes[b]) {
            std::swap(a, b);
        }
    
        parents[b] = a;
        sizes[a] += sizes[b];
    }

    return *this;
}

size_t UnionFind::num_nodes() const {
    return parents.size();
}

size_t UnionFind::subtree_size(const uf_index_t x) const {
    return sizes[x];
}

// UNION FIND COLLECTOR ACCESSOR

using Accessor = UnionFindCollector::Accessor;

int32_t Accessor::component_of(const int32_t flat_index) const {
    return counts[sorted_mappings[flat_index]];
}

UnionFind::uf_index_t Accessor::to_uf(const int32_t flat_index) const {
    return sorted_mappings[flat_index];
}

size_t Accessor::size() const {
    return counts.size();
}

// UNION FIND COLLECTOR

size_t UnionFindCollector::size() const {
    return accessor.counts.size();
}

int32_t UnionFindCollector::component_of(const int32_t flat_index) const {
    return accessor.component_of(flat_index);
}

UnionFind::uf_index_t UnionFindCollector::to_uf(const int32_t flat_index) const {
    return accessor.to_uf(flat_index);
}

UnionFindCollector& UnionFindCollector::fit(const UnionFind& uf) {
    const int32_t N = (int32_t) uf.num_nodes();
    if (N != size()) {
        accessor.counts = std::vector<int32_t>(N);
        accessor.sorted_mappings = std::vector<int32_t>(N);
    }

    int32_t sum = 0;
    for (int32_t i = 0; i < N; i++) {
        const int32_t count = (int32_t) uf.is_root(i) * (int32_t) uf.subtree_size(i);
        accessor.counts[i] = sum;
        sum += count;
    }

    for (int32_t i = 0; i < N; i++) {
        const int32_t root = uf.find(i);
        int32_t& root_count = accessor.counts[root];
        accessor.sorted_mappings[root_count] = i;
        root_count += 1;
    }

    return *this;
}

UnionFindCollector::UnionFindCollector() : accessor() {}

UnionFindCollector::UnionFindCollector(const UnionFind& uf) 
    : accessor(std::vector<int32_t>(uf.num_nodes(), 0), std::vector<uf_index_t>(uf.num_nodes(), 0)) {
    fit(uf);
}

UnionFindCollector::ComponentRangeView UnionFindCollector::components() const & {
    return ComponentRangeView(*this);
}

UnionFindCollector::ComponentRangeOwned UnionFindCollector::components() && {
    return ComponentRangeOwned(std::move(*this));
}

using ComponentRangeIterator = UnionFindCollector::ComponentRangeIterator;
using ComponentGroup = UnionFindCollector::ComponentGroup;
using ComponentRangeView = UnionFindCollector::ComponentRangeView;
using ComponentRangeOwned = UnionFindCollector::ComponentRangeOwned;

// COMPONENT RANGE ITERATOR

bool ComponentRangeIterator::component_equals(const int32_t a, const int32_t b) const {
    return accessor.component_of(a) == accessor.component_of(b);
}

ComponentRangeIterator& ComponentRangeIterator::advance() {
    while (cur_index < accessor.size() && component_equals(cur_index, component_start_index)) {
        cur_index += 1;
    }
    return *this;
}

ComponentRangeIterator::ComponentRangeIterator(const Accessor& a, int32_t start)
    : accessor(a), cur_index(start) {
    advance();
}

ComponentRangeIterator::ComponentRangeIterator(const Accessor& a, int32_t start, int32_t component_start) 
    : accessor(a), cur_index(start), component_start_index(component_start) {}

ComponentGroup ComponentRangeIterator::operator*() const {
    return ComponentGroup {
        component_start_index,
        cur_index
    };
}

ComponentRangeIterator& ComponentRangeIterator::operator++() {
    component_start_index = cur_index;
    return advance();
}

ComponentRangeIterator ComponentRangeIterator::operator++(int) {
    ComponentRangeIterator dupe = ComponentRangeIterator(accessor, cur_index, component_start_index);
    ++(*this);
    return dupe;
}

bool ComponentRangeIterator::operator==(const ComponentRangeIterator& other) const {
    return component_start_index == other.component_start_index && cur_index == other.cur_index;
}

bool ComponentRangeIterator::operator!=(const ComponentRangeIterator& other) const {
    return !(*this == other);
}

// COMPONENT RANGE

ComponentRangeView::ComponentRangeView(const UnionFindCollector& u) : accessor(accessor) {}

ComponentRangeView::iterator ComponentRangeView::begin() {
    return iterator(accessor, 0);
}

ComponentRangeView::iterator ComponentRangeView::end() {
    return iterator(accessor, (int32_t) accessor.size(), (int32_t) accessor.size());
}

// COMPONENT RANGE OWNED

ComponentRangeOwned::ComponentRangeOwned(UnionFindCollector&& u)
    : accessor(std::move(u.accessor.counts), std::move(u.accessor.sorted_mappings)) {}

ComponentRangeOwned::iterator ComponentRangeOwned::begin() {
    return iterator(accessor, 0);
}

ComponentRangeOwned::iterator ComponentRangeOwned::end() {
    return iterator(accessor, (int32_t) accessor.size(), (int32_t) accessor.size());
}
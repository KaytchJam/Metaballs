#include <dsa/unionfind.hpp>
#include <cstring>
#include <iostream>

using namespace gtt::dsa;


// UNION FIND

UnionFind::UnionFind(size_t n) : parents(n), sizes(n,1), stack() {
    for (uf_index_t i = 0; i < (uf_index_t) n; i++) {
        parents[i] = i;
    }
    stack.reserve(n);
}

UnionFind::~UnionFind() {}

UnionFind& UnionFind::pop_unsafe() {
    parents.pop_back();
    sizes.pop_back();
    return *this;
}

const UnionFind& UnionFind::debug_print() const {
    std::cout << "UnionFind::debug_print::[";
    size_t N = parents.size();

    if (N > 0) {
        std::cout << "(P:" << parents[0] << ",S:" << sizes[0] << ")";
    }

    for (size_t i = 1; i < parents.size(); i++) {
        std::cout << ", (P:" << parents[i] << ",S:" << sizes[i] << ")";
    }

    std::cout << "]" << std::endl;
    return *this;
}

UnionFind::uf_index_t UnionFind::find_mut(const uf_index_t x) {
    if (parents[x] == x) {
        return x;
    }

    // Discovery phase
    uf_index_t current = x;
    while (parents[current] != current) {
        stack.push(StackFrame(false, current));
        current = parents[current];
    }
    
    // Backtrack and compress paths
    uf_index_t top_parent = current;
    while (!stack.empty()) {
        StackFrame& frame = stack.top();
        parents[frame.index] = top_parent;
        stack.pop();
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

UnionFind& UnionFind::add_vertex() {
    parents.push_back((int32_t) num_nodes());
    sizes.push_back(1);
    return *this;
}

UnionFind& UnionFind::reset_remove() {
    reset();
    parents.pop_back();
    sizes.pop_back();
    return *this;
}

UnionFind& UnionFind::remove_vertex(const uf_index_t x) {
    const int32_t end_idx = (int32_t) parents.size() - 1;
    isolate(x).update(end_idx, x);

    std::swap(parents[x], parents[end_idx]);
    std::swap(sizes[x], sizes[end_idx]);

    parents.pop_back();
    sizes.pop_back();

    return *this;
}

UnionFind& UnionFind::reset() {
    // std::printf("\nUnionFind::reset()\n\n");
    const int32_t N = (int32_t) parents.size();
    for (int32_t i = 0; i < N; i++) { parents[i] = i; }
    std::fill(sizes.begin(), sizes.end(), 1);
    return *this;
}

size_t UnionFind::num_nodes() const {
    return parents.size();
}

size_t UnionFind::subtree_size(const uf_index_t x) const {
    return sizes[x];
}

const std::vector<int32_t>& UnionFind::get_parents() const {
    return parents;
}

UnionFind& UnionFind::update(const uf_index_t a, const uf_index_t b) {
    const int32_t N = (int32_t) num_nodes();
    for (int32_t i = 0; i < N; i++) {
        if (parents[i] == a) {
            parents[i] = b;
        }
    }

    parents[a] = b;
    return *this;
}

UnionFind& UnionFind::isolate(const uf_index_t a) {
    const int32_t N = (int32_t) num_nodes();
    for (int32_t i = 0; i < N; i++) {
        if (parents[i] == a) {
            parents[i] = parents[a];
        }
    }

    /** Travel upwards, reduce sizes */
    sizes[a] = 1;
    uf_index_t p = parents[a];
    while (p != parents[p]) {
        sizes[p] -= 1;
        p = parents[p];
    }

    parents[a] = a;
    return *this;
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
        accessor.counts = std::vector<int32_t>(N, 0);
        accessor.sorted_mappings = std::vector<int32_t>(N, 0);
    }

    // std::cout << "UnionFindCollector:: N = " << N << std::endl;

    // for (int i = 0; i < N; i++) {
    //     std::printf("UnionFindCollector::fit:: find(%d) = %d\n", i, (int32_t) uf.get_parents()[i]);
    // }
 
    int32_t sum = 0;
    for (int32_t i = 0; i < N; i++) {
        // std::printf("UnionFindCollector::fit:: subtree_size(%d) = %d\n", i, (int32_t) uf.subtree_size(i));
        const int32_t count = (int32_t) uf.is_root(i) * (int32_t) uf.subtree_size(i);
        accessor.counts[i] = sum;
        sum += count;
    }
    
    // std::cout << "ACCESSOR::COUNTS(1)::[";
    // for (int i = 0; i < N; i++) {
    //     std::cout << accessor.counts[i] << ",";
    // }
    // std::cout << "]" << std::endl;

    for (int32_t i = 0; i < N; i++) {
        const int32_t root = uf.find(i);
        int32_t& root_count = accessor.counts[root];
        accessor.sorted_mappings[root_count] = i;
        root_count += 1;
    }

    // std::cout << "ACCESSOR::SORTED_MAPPINGS::[";
    // for (int i = 0; i < N; i++) {
    //     std::cout << accessor.sorted_mappings[i] << ",";
    // }
    // std::cout << "]" << std::endl;


    for (int32_t i = 0; i < N; i++) {
        accessor.counts[i] = uf.find(i);
    }

    // std::cout << "ACCESSOR::COUNTS(2)::[";
    // for (int i = 0; i < N; i++) {
    //     std::cout << accessor.counts[i] << ",";
    // }
    // std::cout << "]" << std::endl;

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

const UnionFindCollector::Accessor& UnionFindCollector::get_accessor() const & {
    return accessor;
}

UnionFindCollector::Accessor UnionFindCollector::get_accessor() && {
    return accessor;
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

ComponentRangeView::ComponentRangeView(const UnionFindCollector& u) : accessor(u.accessor) {}

ComponentRangeView::iterator ComponentRangeView::begin() const {
    return iterator(accessor, 0);
}

ComponentRangeView::iterator ComponentRangeView::end() const {
    return iterator(accessor, (int32_t) accessor.size(), (int32_t) accessor.size());
}

// COMPONENT RANGE OWNED

ComponentRangeOwned::ComponentRangeOwned(UnionFindCollector&& u)
    : accessor(std::move(u.accessor.counts), std::move(u.accessor.sorted_mappings)) {}

ComponentRangeOwned::iterator ComponentRangeOwned::begin() const {
    return iterator(accessor, 0);
}

ComponentRangeOwned::iterator ComponentRangeOwned::end() const {
    return iterator(accessor, (int32_t) accessor.size(), (int32_t) accessor.size());
}
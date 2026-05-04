#pragma once

#include <dsa/indexstack.hpp>
#include <dsa/wrappers.hpp>

#include <vector>
#include <cstdint>

namespace gtt {
    namespace dsa {

        /** Impelmentation of a Disjoint Set Union, defined over a fixed number
         * of elements / vertices N. */
        struct UnionFind {
            public:
                using uf_index_t = int32_t;
                using StackFrame = wrap::IndexWrapper<bool>;
            private:
                std::vector<uf_index_t> parents;
                std::vector<int32_t> sizes;
                IndexedStack<StackFrame> stack;
            public:
                UnionFind(const size_t n);
                ~UnionFind();

                uf_index_t find(const uf_index_t x);                        // find the representative node of the set containing `x`
                UnionFind& unite(const uf_index_t a, const uf_index_t b);   // join the sets containing nodes `a` and `b`

                inline bool is_root(const uf_index_t x) const;              // Returns whether node `x` is a root node or not
                inline size_t num_nodes() const;                            // Returns the total number of nodes in this union find struct
                inline size_t subtree_size(const uf_index_t x) const;       // Returns the size of the subtree lead by `x`.
        };

        /** Data structure for collecting sets assigned via Union Find. Internally performs
         * a counting sort to group vertices with the same Union Find Set Representative
         * together.
         * 
         * Use `UnionFindCollector.fit(u)` to bind this collector to a Union Find object,
         * and `UnionFindCollector.components()` to get a range over each Union Find set. */
        struct UnionFindCollector {
            std::vector<int32_t> counts;
            std::vector<int32_t> flat;

            inline size_t size() const {
                return counts.size();
            }

            inline int32_t component_of(const int32_t flat_index) {
                return counts[flat[flat_index]];
            }

            struct ComponentGroup {
                const int32_t start;
                const int32_t end;
            };

            struct ComponentRangeIterator {
                int32_t cur_index = 0;
                int32_t component_start_index = 0;
                UnionFindCollector& ufc;

                ComponentRangeIterator& advance() {
                    while (cur_index < ufc.size() && ufc.component_of(cur_index) == ufc.component_of(component_start_index)) {
                        cur_index += 1;
                    }
                    return *this;
                }

                ComponentRangeIterator(UnionFindCollector& u, int32_t start) : ufc(u), cur_index(start) {
                    advance();
                }

                ComponentRangeIterator(UnionFindCollector& u, int32_t start, int32_t component_start) 
                    : ufc(u), cur_index(start), component_start_index(component_start) {}

                using value_type = ComponentGroup;
                using reference = void;
                using pointer = void;
                using difference_type = std::ptrdiff_t;
                using iterator_category = std::forward_iterator_tag;
                
                ComponentGroup operator*() const {
                    return ComponentGroup {
                        component_start_index,
                        cur_index
                    };
                }

                ComponentRangeIterator& operator++() {
                    component_start_index = cur_index;
                    return advance();
                }

                ComponentRangeIterator operator++(int) {
                    ComponentRangeIterator dupe = ComponentRangeIterator(ufc, cur_index, component_start_index);
                    (*this)++;
                    return dupe;
                }

                bool operator==(const ComponentRangeIterator& other) const {
                    return component_start_index == other.component_start_index && cur_index == other.cur_index;
                }

                bool operator!=(const ComponentRangeIterator& other) const {
                    return !(*this == other);
                }
            };

            /** Simple Range object. */
            struct ComponentRange {
                UnionFindCollector& ufc;
                ComponentRange(UnionFindCollector& u) : ufc(u) {}

                using iterator = ComponentRangeIterator;
                iterator begin() { return iterator(ufc, 0); }
                iterator end() { return iterator(ufc, (int32_t) ufc.size(), (int32_t) ufc.size()); }
            };

            UnionFindCollector& fit(UnionFind& uf) {
                const int32_t N = (int32_t) uf.num_nodes();
                if (N != size()) {
                    counts = std::vector<int32_t>(N);
                    flat = std::vector<int32_t>(N);
                }

                int32_t sum = 0;
                for (int32_t i = 0; i < N; i++) {
                    const int32_t count = (int32_t) uf.is_root(i) * uf.subtree_size(i);
                    counts[i] = sum;
                    sum += count;
                }

                for (int32_t i = 0; i < N; i++) {
                    const int32_t root = uf.find(i);
                    int32_t& root_count = counts[root];
                    flat[root_count] = i;
                    root_count += 1;
                }

                return *this;
            }

            UnionFindCollector(UnionFind& uf) 
                : counts(uf.num_nodes(), 0), flat(uf.num_nodes(), 0) {
                fit(uf);
            }

            /** Returns a range that iterates over each set in the fitted UnionFind */
            ComponentRange components() {
                return ComponentRange(*this);
            }
        };
    }
}
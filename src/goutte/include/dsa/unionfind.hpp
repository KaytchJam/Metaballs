#pragma once

#include <dsa/indexstack.hpp>
#include <dsa/wrappers.hpp>

#include <vector>
#include <cstdint>
#include <iterator>

namespace gtt {
    namespace dsa {

        /** Implementation of a Disjoint Set Union, defined over a fixed number
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

                uf_index_t find_mut(const uf_index_t x);                    // Find the representative node of the set containing `x`.
                uf_index_t find(const uf_index_t x) const;                  // Find the representative node of the set containing `x`.
                UnionFind& unite(const uf_index_t a, const uf_index_t b);   // Join the sets containing nodes `a` and `b`.
                UnionFind& add_vertex();                                    // Adds a new set index 'N' to a UnionFind of size N
                UnionFind& remove_vertex(const uf_index_t a);               // Removes vertex at index a via swap & pop. 
                UnionFind& reset_remove();                                  // Reset & pop back the UnionFind 
                UnionFind& reset();                                         // Reset the UnionFind sets to all be disjoint
                UnionFind& isolate(const uf_index_t a);                     // Make node `a` disjoint. 
                UnionFind& update(const uf_index_t a, const uf_index_t b);  // Update all nodes n with parent(n) = a to parent(n) = b
                UnionFind& pop_unsafe();                                    // Reduce the size of the UnionFind and make no other modifications

                inline bool is_root(const uf_index_t x) const;              // Returns whether node `x` is a root node or not.
                inline size_t num_nodes() const;                            // Returns the total number of nodes in this union find struct.
                inline size_t subtree_size(const uf_index_t x) const;       // Returns the size of the subtree lead by `x`.
                inline const std::vector<uf_index_t>& get_parents() const;
        };

        /** Data structure for collecting sets assigned via Union Find. Internally performs
         * a counting sort to group vertices with the same Union Find Set Representative
         * together.
         * 
         * Use `UnionFindCollector.fit(u)` to bind this collector to a Union Find object,
         * and `UnionFindCollector.components()` to get a range over each Union Find set. */
        struct UnionFindCollector {
            public:
                typedef UnionFind::uf_index_t uf_index_t;

                struct Accessor {
                    std::vector<int32_t> counts;
                    std::vector<uf_index_t> sorted_mappings;

                    int32_t component_of(const int32_t flat_index) const;
                    uf_index_t to_uf(const int32_t flat_index) const;
                    inline size_t size() const;
                };

            private:
                Accessor accessor;
            public:
                inline size_t size() const;
                inline int32_t component_of(const int32_t flat_index) const;
                inline uf_index_t to_uf(const int32_t flat_index) const;

                /** Simple indexable range representing  */
                struct ComponentGroup {
                    const int32_t start;
                    const int32_t end;
                };

                struct ComponentRangeIterator {
                    private:
                        int32_t cur_index = 0;
                        int32_t component_start_index = 0;
                        
                        const Accessor& accessor;

                        /** Checks if two nodes are in the same component */
                        inline bool component_equals(const int32_t a, const int32_t b) const;
                        ComponentRangeIterator& advance();

                    public:
                        ComponentRangeIterator(const Accessor& u, int32_t start);
                        ComponentRangeIterator(const Accessor& u, int32_t start, int32_t component_start);

                        using value_type = ComponentGroup;
                        using reference = void;
                        using pointer = void;
                        using difference_type = std::ptrdiff_t;
                        using iterator_category = std::forward_iterator_tag;
                        
                        ComponentGroup operator*() const;
                        ComponentRangeIterator& operator++();
                        ComponentRangeIterator operator++(int);
                        bool operator==(const ComponentRangeIterator& other) const;
                        bool operator!=(const ComponentRangeIterator& other) const;
                };

                /** Non-owning view of the UnionFindCollector */
                struct ComponentRangeView {
                    private:
                        const Accessor& accessor;
                    public:
                        ComponentRangeView(const UnionFindCollector& u);
                        using iterator = ComponentRangeIterator;
                        iterator begin() const;
                        iterator end() const;
                };

                /** Owning view of the UnionFindCollector */
                struct ComponentRangeOwned {
                    private:
                        const Accessor accessor;
                    public:
                        ComponentRangeOwned(UnionFindCollector&& u);
                        using iterator = ComponentRangeIterator;
                        iterator begin() const;
                        iterator end() const;
                };
            
                UnionFindCollector();
                UnionFindCollector(const UnionFind& uf);

                UnionFindCollector& fit(const UnionFind& uf);
                ComponentRangeView components() const &;
                ComponentRangeOwned components() &&;

                const Accessor& get_accessor() const &;
                Accessor get_accessor() &&;
        };
    }
}

// GOUTTE COMMON
#include <common/lalg.hpp>
#include <common/wrappers.hpp>

// GOUTTE
#include <isosurface.hpp>
#include <fieldrange.hpp>
#include <aabb_tree.hpp>
#include <metaball_presets.hpp>

// STDLIB
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

template <typename T>
struct IndexedStack {
    std::vector<T> container;
    size_t stack_size = 0;

    IndexedStack reserve(size_t size) {
        container.reserve(size);
        return *this;
    }

    /** Push an item to the stack */
    IndexedStack& push(const T& t) {
        if (stack_size >= container.size()) {
            container.push_back(t);
        } else {
            container[stack_size] = t;
        }

        stack_size += 1;
        return *this;
    }

    // Pop an item off the stack, and return it by value
    T pop() {
        stack_size -= (int32_t) (stack_size > 0) *  1;
        return container[stack_size];
    }

    // Get the item at the top of the stack
    T& top() {
        return container[stack_size - 1];
    }

    // Return whether the stack is empty or not
    bool empty() const {
        return stack_size == 0;
    }

    // Reset the stack
    IndexedStack& reset() {
        stack_size = 0;
        return *this;
    }
};

typedef int32_t uf_index;

/** Impelmentation of a Disjoint Set Union, defined over a fixed number
 * of elements / vertices N. */
struct UnionFind {
    std::vector<uf_index> parents;
    std::vector<int32_t> sizes;
    
    using StackFrame = gtt::common::wrap::IndexWrapper<bool>;

    // For the use of mimicking recursion. Allocated once for reuse
    // in the various function calls.
    IndexedStack<StackFrame> smoke;
    
    UnionFind(const size_t n) : parents(n), sizes(n,1), smoke() {
        for (size_t i = 0; i < n;i++) {
            parents[i] = (int32_t) i;
        }

        smoke.reserve(n);
    }

    /** Finds the 'representative' of the set containing
     * `uf_index` x. */
    uf_index find(const uf_index x) {
        uf_index top_parent = parents[x];

        smoke.push(StackFrame(false, x));
        while (!smoke.empty()) {
            StackFrame& frame = smoke.top();

            // backtracking
            if (*frame) {
                parents[frame.index] = top_parent;
                smoke.pop();

            // first visit
            } else {
                frame.item = true;
                if (parents[frame.index] == frame.index) {
                    top_parent = frame.index;
                    smoke.pop();
                }
            }
        }

        smoke.reset();
        return top_parent;
    }

    bool is_root(const uf_index x) const {
        return x == parents[x];
    }

    /** Combines the set containing `uf_index a` and the set containing `uf_index b`. */
    void unite(uf_index a, uf_index b) {
        a = find(a);
        b = find(b);

        if (a == b) {
            return;
        }

        if (sizes[a] < sizes[b]) {
            std::swap(a, b);
        }

        parents[b] = a;
        sizes[a] += sizes[b];
    }

    size_t size() const {
        return parents.size();
    }
};

// template <typename F, typename T>
// concept FoldFunc = requires(F f, T acc, int32_t i) {
//     { f(acc, i) } -> std::same_as<T>;
// };

/** Data structure for collecting sets assigned via Union Find. */
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
            std::cout << "component_start_index = " << component_start_index << ", cur_index = " << cur_index << std::endl;
            return ComponentGroup {
                component_start_index,
                cur_index
            };
        }

        ComponentRangeIterator& operator++() {
            component_start_index = std::max(component_start_index, cur_index);
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

    struct ComponentRange {
        UnionFindCollector& ufc;
        ComponentRange(UnionFindCollector& u) : ufc(u) {}

        using iterator = ComponentRangeIterator;
        iterator begin() { return iterator(ufc, 0); }
        iterator end() { return iterator(ufc, (int32_t) ufc.size(), (int32_t) ufc.size()); }
    };

    UnionFindCollector& fit(UnionFind& uf) {
        if (uf.size() != size()) {
            counts = std::vector<int32_t>(uf.size());
            flat = std::vector<int32_t>(uf.size());
        }

        const int32_t N = (int32_t) size();
        int32_t sum = 0;
        for (int32_t i = 0; i < N; i++) {
            const int32_t count = (int32_t) uf.is_root(i) * uf.sizes[i];
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

    UnionFindCollector() : counts(), flat() {}

    UnionFindCollector(UnionFind& uf) 
        : counts(uf.parents.size(), 0), flat(uf.parents.size(), 0) {
        fit(uf);
    }

    ComponentRange components() {
        return ComponentRange(*this);
    }
};

/** Implementation 1 for coalescing Bounding Boxes in an AABB Tree. Passes indices into 
 * a fixed size buffer and tries to use the spatial locality of the tree traversal to 
 * insert indices into component-sorted order. Unfortunately this approach fails in
 * correctness! */
int naive_1() {
    gtt::AABBTree tree;

    using KB = gtt::presets::KineticBlob;

    KB blobs[] {
        KB(),
        KB(gtt::lalg::vec3(1.f)),
        KB(gtt::lalg::vec3(-2.f)),
        KB(gtt::lalg::vec3(-5.f)),
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f)
    };

    int N = sizeof(blobs) / sizeof(KB);

    {
        int index = 0;
        for (auto& i : blobs) {
            tree.insert(index, i.get_bounding_box());
            index += 1;
        }
    }

    std::cout << "Iterating through all nodes" << std::endl;

    for (int i = 0; i < tree.nodes.size(); i++) {
        gtt::AABBNode& n = tree.nodes[i];
        BoundingBox& bb = n.bb;
        std::cout << "BOUNDING BOX #" << i << ":" <<  ( i == tree.root ? " (ROOT!)" : "") << std::endl;
        std::cout << "\tMAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")\n";
        std::cout << "\tMIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")\n";
        std::cout << "\tLEFT: " << n.left << "\n\tRIGHT: " << n.right << "\n\tPARENT: " << n.parent << std::endl;
    }

    std::cout << "Print all leaves: " << std::endl;
    std::unordered_map<int32_t, int32_t> color;
    color.reserve(N);

    using IWrapper = gtt::common::wrap::IndexWrapper<int32_t>;
    std::vector<IWrapper> blob_indices;
    blob_indices.reserve(N);

    // UnionFind uf(tree.nodes.size());
    int global_color = 0;
    tree.all_overlaps([&color,&global_color, &blob_indices, &tree](int32_t a, int32_t b) {
        int color_local = global_color;
        const int a_blob_index = tree.nodes[a].data_index;
        const int b_blob_index = tree.nodes[b].data_index;

        const bool has_a = color.find(a_blob_index) != color.end();
        const bool has_b = color.find(b_blob_index) != color.end();

        if (has_a && has_b) {
            color_local = std::min(color[a_blob_index], color[b_blob_index]);
            color[a_blob_index] = color_local;
            color[b_blob_index] = color_local;
        } else if (has_a) {
            color_local = color[a_blob_index];
            color[b_blob_index] = color_local;

            blob_indices.push_back(IWrapper(color_local, b_blob_index));
        } else if (has_b) {
            color_local = color[b_blob_index];
            color[a_blob_index] = color_local;

            blob_indices.push_back(IWrapper(color_local, a_blob_index));
        } else {
            color[a_blob_index] = color_local;
            color[b_blob_index] = color_local;
            global_color += 1;

            blob_indices.push_back(IWrapper(color_local, a_blob_index));
            blob_indices.push_back(IWrapper(color_local, b_blob_index));
        }
    });

    for (int i = 0; i < N; i++) {
        if (color.find(i) == color.end()) {
            blob_indices.push_back(IWrapper(-1, i));
        }

        const IWrapper& iw = blob_indices[i];
        std::cout << "(BLOB INDEX=" << i << ", COLOR=" << *iw << ")" << std::endl;
    }

    struct UnionBB {
        BoundingBox bb;
        IntRange ir;
    };

    std::vector<UnionBB> bbs;
    bbs.reserve(N);

    int32_t last_color = -1;
    int32_t start_index = -1;
    BoundingBox bb;

    int32_t cur_index = 0;
    for (const IWrapper& iw : blob_indices) {
        if (*iw != last_color || last_color == -1) {
            if (start_index >= 0) {
                bbs.push_back(UnionBB{ bb, IntRange(start_index, cur_index) });
                // this data structure isn't actually needed, I believe we can do this lazily
            }

            start_index = cur_index;
            last_color = *iw;
            bb = blobs[iw.index].get_bounding_box();
        } else {
            bb = gtt::join(bb, blobs[iw.index].get_bounding_box());
        }

        cur_index += 1;
    }

    if (blob_indices.size() > 0) {
        bbs.push_back(UnionBB { bb, IntRange(start_index, (int32_t) blob_indices.size())});
    }

    int bb_index = 0;
    for (UnionBB& ub : bbs) {
        std::cout << "Bounding Box index = " << bb_index;

        std::cout << ",\tBall Indices: [";
        for (int i : ub.ir) {
            std::cout << i << " ";
        }
        std::cout << "], ";

        BoundingBox& bb = ub.bb;
        std::cout << "\tMAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")";
        std::cout << "\tMIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")\n";
        bb_index += 1;
    }

    return EXIT_SUCCESS;
}


using KB = gtt::presets::KineticBlob;

template <typename T, size_t N>
using Arr = T[N];

typedef int32_t metaball_index;
typedef int32_t aabbnode_index;
typedef int32_t component_number;

struct ComponentWrapper {
    component_number component;
    metaball_index m_idx;

    template <size_t N>
    KB& get_metaball(Arr<KB,N>& container) {
        return container[m_idx];
    }
};

std::string to_string(const BoundingBox& bb) {
    std::stringstream ss;
    ss << "[MAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")" 
        << ", MIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")]";
    return ss.str();
}

/** Impelemntation 2 for coalescing overlapping bounding boxes. It utilizes a graph
 * data structure and then uses DFS to find all Connected Components */
int naive_2() {
    // Our List of (Kinetic) Metaballs
    KB blobs[] = {
        KB(),
        KB(gtt::lalg::vec3(1.f)),
        KB(gtt::lalg::vec3(-2.f)),
        KB(gtt::lalg::vec3(-5.f)),
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f)
    };

    const size_t num_metaballs = sizeof(blobs) / sizeof(KB);

    std::cout << "Insert into AABBTree" << std::endl;
    // Insert into the AABBTree
    gtt::AABBTree tree;
    int i = 0;
    for (KB& blob : blobs) {
        tree.insert(i, blob.get_bounding_box());
        i += 1;
    }

    std::cout << "Build graph" << std::endl;
    // BASIC IDEA: Build a graph for our metaballs, edges are metaballs w/ overlapping bounding boxes
    std::unordered_map<metaball_index, std::unordered_set<metaball_index>> graph; // metaball index + edge head index

    tree.all_overlaps([&tree, &graph](aabbnode_index a, aabbnode_index b) {
        const metaball_index ma = tree.nodes[a].data_index;
        const metaball_index mb = tree.nodes[b].data_index;
        
        if (graph.find(ma) == graph.end()) { graph[ma] = {};}
        if (graph.find(mb) == graph.end()) { graph[mb] = {};}

        graph[ma].insert(mb);
        graph[mb].insert(ma);
    });

    std::cout << "Printing graph" << std::endl;
    for (auto p : graph) {
        std::cout << p.first << " : { ";
        for (auto q : p.second) {
            std::cout << q << " ";
        }
        std::cout << "}" << std::endl;
    }
    
    // Find Connected Components:
    std::cout << "Begin Connected Components Search" << std::endl;
    std::unordered_set<metaball_index> visited;
    std::vector<ComponentWrapper> components_buffer;
    components_buffer.reserve(num_metaballs);

    IndexedStack<metaball_index> bag = IndexedStack<metaball_index>().reserve(num_metaballs);
    metaball_index entry = 0;
    component_number current_component = 0;

    while (visited.size() < num_metaballs) {

        // Increment until we find a metaball_index not used yet. This will be
        // our entry point into our DFS

        std::cout << "Entry search: " << visited.size() << std::endl;
        while (visited.find(entry) != visited.end()) {
            entry += 1;
        }

        // DFS
        std::cout << "DFS" << std::endl;
        bag.push(entry);
        while (!bag.empty()) {
            const metaball_index cur = bag.pop();
            if (visited.find(cur) == visited.end()) {
                visited.insert(cur);
                components_buffer.push_back(ComponentWrapper{current_component, cur});

                for (metaball_index neighbor : graph[cur]) {
                    bag.push(neighbor);
                }
            }
        }
        
        current_component += 1;
        bag.reset();
    }

    std::cout << "Coalesce metaballs" << std::endl;

    // Coalesce Metaballs in the same component
    component_number prev_comp = -1;
    int32_t last_index = -1;
    int32_t current_index = 0;
    BoundingBox bb;

    for (ComponentWrapper& cw : components_buffer) {
        if (cw.component != prev_comp || prev_comp == -1) {
            if (last_index != -1) {
                std::cout << to_string(bb) << ", [ ";
                for (metaball_index m : IntRange(last_index, current_index)) { std::cout << m << " "; }
                std::cout << "], Component = " << cw.component << std::endl;
            }

            last_index = current_index;
            bb = cw.get_metaball(blobs).get_bounding_box();
            prev_comp = cw.component;
        } else {
            bb = gtt::join(bb, cw.get_metaball(blobs).get_bounding_box());
        }

        current_index += 1;
    }

    if (current_index >= 0) {
        std::cout << to_string(bb) << ", [ ";
        for (metaball_index m : IntRange(last_index, current_index)) { std::cout << m << " "; }
        std::cout << "]" << std::endl;
    }

    return EXIT_SUCCESS;
}

/** Implementation 3 of coalescing overlapping bounding boxes in an AABB Tree. This 3rd implementation
 * uses a Disjoint Set Union. */
int naive_3() {
    // Our List of (Kinetic) Metaballs
    KB blobs[] = {
        KB(),
        KB(gtt::lalg::vec3(1.f)),
        KB(gtt::lalg::vec3(-2.f)),
        KB(gtt::lalg::vec3(-5.f)),
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f)
    };

    const size_t num_metaballs = sizeof(blobs) / sizeof(KB);

    std::cout << "Insert into AABBTree" << std::endl;
    // Insert into the AABBTree
    gtt::AABBTree tree;
    int i = 0;
    for (KB& blob : blobs) {
        tree.insert(i, blob.get_bounding_box());
        i += 1;
    }

    UnionFind uf(num_metaballs);
    tree.all_overlaps([&tree, &uf](aabbnode_index a, aabbnode_index b) {
        uf.unite(tree.nodes[a].data_index, tree.nodes[b].data_index);
    });

    UnionFindCollector ufc(uf);

    // std::cout << "FLAT = ";
    // for (int i = 0; i < ufc.size(); i++) {
    //     std::cout << ufc.flat[i] << " ";
    // }
    // std::cout << "\nCOUNTS = ";
    // for (int i = 0; i < ufc.size(); i++) {
    //     std::cout << ufc.counts[i] << " ";
    // }
    // std::cout << std::endl;

    for (UnionFindCollector::ComponentGroup group : ufc.components()) {
        std::cout << "SET = [ ";
        for (int i = group.start; i < group.end; i++) {
            std::cout << i << " ";
        }
    }


    // component_number prev_comp = -1;
    // int32_t last_index = -1;
    // BoundingBox bb;

    // for (int i = 0; i < ufc.size(); i++) {
    //     const metaball_index m = ufc.flat[i];
    //     const component_number c = ufc.counts[m];

    //     if (c != prev_comp || prev_comp == -1) {
    //         if (last_index != -1) {
    //             std::cout << to_string(bb) << ", [ ";
    //             for (metaball_index m : IntRange(last_index, i)) { std::cout << m << " "; }
    //             std::cout << "], Component = " << ufc.counts[ufc.flat[last_index]] << std::endl;
    //         }

    //         last_index = i;
    //         bb = blobs[m].get_bounding_box();
    //         prev_comp = c;
    //     } else {
    //         bb = gtt::join(bb, blobs[m].get_bounding_box());
    //     }
    // }

    // if (ufc.size() > 0) {
    //     std::cout << to_string(bb) << " : [ ";
    //     for (metaball_index m : IntRange(last_index, (int32_t) ufc.size())) { std::cout << m << " "; }
    //     std::cout << "], Component = " << ufc.counts[ufc.flat[ufc.size() - 1]] << std::endl;
    // }
    return EXIT_SUCCESS;
}

int main() {
    // naive_1();
    naive_3();

    return EXIT_SUCCESS;
}
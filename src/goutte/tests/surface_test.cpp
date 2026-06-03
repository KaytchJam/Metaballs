
// GOUTTE COMMON
#include <common/lalg.hpp>

// GOUTTE DSA
#include <dsa/wrappers.hpp>
#include <dsa/indexstack.hpp>
#include <dsa/unionfind.hpp>

// GOUTTE
#include <isosurface.hpp>
#include <fieldrange.hpp>
#include <aabb_tree.hpp>
#include <metaball_presets.hpp>
#include <metaball_working_set.hpp>

// STDLIB
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <ranges>
#include <span>

typedef int32_t uf_index;

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

    using IWrapper = gtt::dsa::wrap::IndexWrapper<int32_t>;
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

    gtt::dsa::IndexedStack<metaball_index> bag = gtt::dsa::IndexedStack<metaball_index>().reserve(num_metaballs);
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

template <typename>
struct FnHelper;

template <typename Ret, typename... Args>
struct FnHelper<Ret(Args...)> {
    using type = Ret(*)(Args...);
};

template <typename Signature>
using Fn = typename FnHelper<Signature>::type;

template <typename T>
std::string to_string(const gtt::lalg::vec<T,3>& v) {
    std::stringstream ss;
    ss << "(" << v.x << "," << v.y << "," << v.z << ")";
    return ss.str();
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
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f),
        KB(gtt::lalg::vec3(0, 5, -4)),
        KB(gtt::lalg::vec3(6, 2, 4))
    };

    const size_t num_metaballs = sizeof(blobs) / sizeof(KB);

    // Insert all nodes into the AABBTree
    gtt::AABBTree tree;
    int i = 0;
    for (KB& blob : blobs) {
        tree.insert(i, blob.get_bounding_box());
        i += 1;
    }

    gtt::dsa::UnionFind uf(num_metaballs);
    tree.all_overlaps([&tree, &uf](aabbnode_index a, aabbnode_index b) {
        uf.unite(tree.nodes[a].data_index, tree.nodes[b].data_index);
    });

    typedef int32_t mball_idx_t;

    struct MetaballRenderingGroup {
        BoundingBox bb = BoundingBox::empty();
        std::span<const mball_idx_t> indices;
    };

    auto collector = gtt::dsa::UnionFindCollector(uf);
    std::vector<MetaballRenderingGroup> groups;

    std::cout << "Populating Groups" << std::endl;
    for (auto component : collector.components()) {
        std::cout << "COMPONENT = (" << component.start << ", " << component.end << ")" << std::endl;
        MetaballRenderingGroup mre;
        mre.indices = std::span(collector.get_accessor().sorted_mappings.data() + component.start, (size_t) component.end - component.start );
        for (int i = component.start; i < component.end; i++) {
            mre.bb = gtt::join(mre.bb, blobs[i].get_bounding_box());
        }

        groups.push_back(mre);
    }

    typedef gtt::IsoSurface IS;
    typedef BoundingBox BB;
    typedef FieldRange FR;

    gtt::IsoSurface is = gtt::IsoSurface::construct(gtt::common::lalg::vec3(0), 10.f, 5);

    const Fn<FR(const IS&, const BB&)> box_to_fieldrange = [](const IS& surface, const BB& bb) {
        const gtt::lalg::ivec3 start = floor(surface.position_to_index(bb.min_point));
        const gtt::lalg::ivec3 end = ceil(surface.position_to_index(bb.max_point));
        return FieldRange({start.x, end.x, start.y, end.y, start.z, end.z});
    };

    std::cout << "Getting Field Ranges" << std::endl;
    int k = 0;
    for (MetaballRenderingGroup& mrg : groups) {
        FieldRange field = box_to_fieldrange(is, mrg.bb);

        std::cout << "BOX #" << k << " = " << to_string(mrg.bb) << " {" << std::endl;
        for (auto idx : field) {
            std::cout << "\t" << to_string(idx) << " = " << to_string(is.get(idx.x, idx.y, idx.z).position) << std::endl;
        }
        std::cout << "}" << std::endl;
        k += 1;
    }

    return EXIT_SUCCESS;
}

int naive_4() {
    using namespace gtt;
    using namespace presets;
    using KB = KineticBlob;

    KB blobs[] = {
        KB(),
        KB(gtt::lalg::vec3(1.f)),
        KB(gtt::lalg::vec3(-2.f)),
        KB(gtt::lalg::vec3(-5.f)),
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f),
        KB(gtt::lalg::vec3(0, 5, -4)),
        KB(gtt::lalg::vec3(6, 2, 4))
    };
    
    gtt::IsoSurface is = gtt::IsoSurface::construct(gtt::common::lalg::vec3(0), 10.f, 5);

    std::cout << "WorkingSet constructor" << std::endl;
    gtt::MetaballWorkingSet<KineticBlob> metaballs(is);

    std::cout << "Inserting metaballs into working set" << std::endl;
    for (KB& kb : blobs) {
        metaballs.add_metaball(std::move(kb));
    }

    std::cout << "Iterating through the groups" << std::endl;
    int group_number = 0;
    for (MetaballWorkingSet<KB>::RenderGroup i : metaballs.groups()) {
        std::cout << "group_number : " << group_number << " = [ ";
        for (int index : i.ball_indices) {
            std::cout << index << " ";
        }
        std::cout << "]" << std::endl;
        group_number += 1;
    }

    return EXIT_SUCCESS;
}

int naive_5() {
    using namespace gtt;
    using namespace presets;
    using BALL = InverseSquareCube;

    BALL blobs[] = {
        BALL(),
        BALL(gtt::lalg::vec3(1.f)),
        BALL(gtt::lalg::vec3(-2.f)),
        BALL(gtt::lalg::vec3(-5.f)),
        BALL(gtt::lalg::vec3(0, 4, -4)),
        BALL(gtt::lalg::vec3(0, 5, -4)),
        BALL(gtt::lalg::vec3(6, 2, 4))
    };
    
    gtt::IsoSurface is = gtt::IsoSurface::construct(gtt::common::lalg::vec3(0), 10.f, 5);

    std::cout << "WorkingSet constructor" << std::endl;
    gtt::MetaballWorkingSet<BALL> metaballs(is);

    std::cout << "Inserting metaballs into working set" << std::endl;
    for (BALL& b : blobs) {
        metaballs.add_metaball(std::move(b));
    }

    std::cout << "Iterating through the groups" << std::endl;
    int group_number = 0;
    for (MetaballWorkingSet<BALL>::RenderGroup i : metaballs.groups()) {
        std::cout << "group_number : " << group_number << " = [ ";
        for (int index : i.ball_indices) {
            std::cout << index << " ";
        }
        std::cout << "] ";

        FieldRange& fr = i.fr;
        std::cout << "START = " << to_string(fr.low()) << ", END = " << to_string(fr.high()) << std::endl;
        group_number += 1;
    }

    return EXIT_SUCCESS;
}

int main() {
    using naive_test = int (*)();
    naive_test tests[] = { naive_1, naive_2, naive_3, naive_4, naive_5 };
    constexpr int test_index = 4;

    constexpr int NUM_TESTS = sizeof(tests) / sizeof(naive_test);

    while (true) {
        bool valid_input = false;
        std::string input;
        int test_num = -1;

        while (!valid_input) {
            std::cout << "\n\nEnter a test number in the range 1-" << NUM_TESTS << ": ";
            std::cin >> input;
            std::cout << "\n";

            std::string_view view(input.data() + input.find_first_not_of(" \t\n!@#$%^&*()-_+=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\\|/{}[]:;\"'<>,.?~`"), 1);

            if (input == "q") {
                std::cout << "Closing program." << std::endl;
                return EXIT_SUCCESS;
            }

            try {
                test_num = std::abs(std::stoi(input));
                if (test_num > NUM_TESTS || test_num <= 0) {
                    std::cout << "Inputted test is outside range. Please enter again... or enter 'q' to quit." << std::endl;
                } else {
                    valid_input = true;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "Error: " << e.what() << std::endl;
                std::cout << "Invalid input. Please enter again... or enter 'q' to quit." << std::endl;
            }
        }

        std::cout << "\n=================================" << std::endl;
        std::cout << "Executing test 'naive_" << (test_num) << "'!\n" << std::endl;
        const int result = tests[test_num - 1]();
        std::cout << "Test ended with status = " << result << std::endl;
        std::cout << "\n=================================" << std::endl;
    }

    return EXIT_SUCCESS;
}
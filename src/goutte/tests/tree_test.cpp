#include <cstdint>
#include <aabb_tree.hpp>
#include <iostream>
#include <sstream>
#include <vector>

using vec3 = gtt::lalg::vec3;
using AABBTree = gtt::AABBTree;
using AABBNode = gtt::AABBNode;

std::string to_string(const BoundingBox& bb) {
    std::stringstream ss;
    ss << "[MAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")" 
        << ", MIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")]";
    return ss.str();
}

/** Just exists for this test. A char and an integer index */
struct CharIdx {
    char c;
    int32_t i;
};

/** Print the tree with mapping information */
void print_tree_with_mappings(const AABBTree& tree, CharIdx* mappings) {
    int32_t idx = 0;
    std::printf("ROOT = %d\n", tree.root);
    for (const AABBNode& n : tree.nodes) {
        std::printf("%d = (%c, P: %d, L: %d, R: %d) -> BB = %s\n", idx, n.data_index >= 0 ? mappings[n.data_index].c : '-', n.parent, n.left, n.right, to_string(n.bb).c_str());
        idx += 1;
    }
}

void print_tree(const AABBTree& tree) {
    int32_t idx = 0;
    for (const AABBNode& n : tree.nodes) {
        std::printf("%d = (P: %d, L: %d, R: %d)\n", idx, n.parent, n.left, n.right);
        idx += 1;
    }
}

/** Remove a value in `mappings` and its corresponding node in the AABBTree. */
bool remove_mapping(std::vector<CharIdx>& mappings, AABBTree& tree, const int32_t remove_idx) {
    if (remove_idx >= mappings.size()) {
        return false;
    }

    /** Delete & update locations in `mappings` */
    gtt::SwapBus locations = tree.remove(mappings[remove_idx].i);
    for (int32_t i = 0; i < locations.count; i++) {
        const int32_t data_location = tree.nodes[locations.records[i].current].data_index;
        mappings[data_location].i = locations.records[i].current;
    }

    /** Swap and pop the corresponding item in `mappings`, and alert the nodes list of 
     * what was swapped around */
    const int32_t end_idx = (int32_t) mappings.size() - 1;
    if (remove_idx != end_idx) {
        tree.nodes[mappings[end_idx].i].data_index = remove_idx;
        std::swap(mappings[remove_idx], mappings[end_idx]);
    }

    mappings.pop_back();
    return true;
}

int main() {
    AABBTree tree(1.5f);
    BoundingBox boxes[] = {
        BoundingBox(vec3(-5, 1, 0), vec3(-3, 5, 0)),
        BoundingBox(vec3(1, -1, 0), vec3(2, 2, 0)),
        BoundingBox(vec3(3, -2, 0), vec3(4, 1, 0))
    };

    std::vector<CharIdx> mappings = {{'A'}, {'B'}, {'C'}};
    int32_t idx = 0;
    for (auto it = std::begin(boxes); it != std::end(boxes); ++it) {
        const int32_t location = tree.insert(idx, *it);
        mappings[idx].i = location;
        idx += 1;
    }

    // Initial Structure of AABB Tree
    print_tree_with_mappings(tree, mappings.data());

    // int32_t removals = 0;
    // while (remove_mapping(mappings, tree, 0)) {
    //     std::cout << "\nRemoval " << removals << std::endl;
    //     print_tree_with_mappings(tree, mappings.data());
    //     removals += 1;
    // }


    std::printf("Callback 'all_overlaps'.\n");
    tree.all_overlaps([&tree](int32_t a, int32_t b) -> void {
        std::printf(
            "%d & %d are overlapping\n", 
            tree.nodes[a].data_index, 
            tree.nodes[b].data_index
        );
    });

    std::printf("Iterator 'all_overlaps'.\n");
    gtt::OverlapTraversal traverse(tree);
    while (traverse.has_next()) {
        gtt::OverlapTraversal::IndexPair pair = traverse.next();
        std::printf(
            "%d & %d are overlapping\n", 
            tree.nodes[pair.first].data_index, 
            tree.nodes[pair.second].data_index
        );
    }

    std::printf("Done.\n");

    return EXIT_SUCCESS;
}
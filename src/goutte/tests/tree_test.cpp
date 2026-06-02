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

struct CharIdx {
    char c;
    int32_t i;
};

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
    AABBTree tree;
    BoundingBox boxes[] = {
        BoundingBox(vec3(-5,1, 0), vec3(-3, 5, 0)),
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

    print_tree_with_mappings(tree, mappings.data());

    int32_t removals = 0;
    while (!mappings.empty()) {
        std::cout << "\nRemoval " << removals << std::endl;
        remove_mapping(mappings, tree, 0);
        print_tree_with_mappings(tree, mappings.data());
        removals += 1;
    }

    return EXIT_SUCCESS;
}
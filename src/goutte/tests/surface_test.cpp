#include <common/lalg.hpp>
#include <isosurface.hpp>
#include <fieldrange.hpp>
#include <aabb_tree.hpp>
#include <metaball_presets.hpp>

#include <iostream>

struct UnionFind {
    std::vector<int32_t> parent;
    std::vector<int32_t> rank;

    UnionFind(size_t n) : parent(n), rank(n,0) {
        for (size_t i=0;i<n;i++) parent[i]=(int32_t)i;
    }

    int32_t find(int32_t x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    }

    void unite(int32_t a, int32_t b) {
        a = find(a);
        b = find(b);
        if (a == b) return;

        if (rank[a] < rank[b]) std::swap(a,b);
        parent[b] = a;

        if (rank[a] == rank[b])
            rank[a]++;
    }
};

int main() {
    gtt::AABBTree tree;

    using KB = gtt::presets::KineticBlob;

    KB blobs[] {
        KB(),
        KB(gtt::lalg::vec3(1.f)),
        KB(gtt::lalg::vec3(-2.f)),
        KB(gtt::lalg::vec3(-5.f)),
        KB(gtt::lalg::vec3(0, 4, -4), gtt::lalg::vec3(0), 2.f)
    };

    for (auto& i : blobs) {
        tree.insert(&i, i.get_bounding_box());
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

    UnionFind uf(tree.nodes.size());
    tree.all_overlaps([&uf](int32_t a, int32_t b) {
        std::cout << a << " overlaps with " << b << std::endl;
        uf.unite(a,b);
    });

    std::unordered_map<int32_t, BoundingBox> merged;

    for (int leaf = 0; leaf < (int) tree.nodes.size(); leaf++) {
        if (tree.nodes[leaf].is_leaf()) {
            int root = uf.find(leaf);
    
            if (!merged.contains(root))
                merged[root] = tree.nodes[leaf].bb;
            else
                merged[root].join_mut(tree.nodes[leaf].bb);
        }
    }

    for (std::pair<const int32_t,BoundingBox>& groups : merged) {
        std::cout << "> " << groups.first << std::endl;
        BoundingBox& bb = groups.second;
        std::cout << "\tMAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")\n";
        std::cout << "\tMIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")\n";
    }

    return EXIT_SUCCESS;
}
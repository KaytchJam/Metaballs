#pragma once

#include <boundingbox.hpp>

namespace gtt {
    /** 
     * Spatial Acceleration data structure that constructs a tree of Axis-Aligned Bounding Boxes.
     */
    struct AABBTree;

    struct AABBNode {
        BoundingBox bb;
        int32_t parent;
    
        union {
            struct {
                int32_t left;
                int32_t right;
            };
            int32_t children[2];
        };

        static AABBNode empty() {
            return AABBNode { BoundingBox(), -1, { -1, -1 } };
        }

        bool is_leaf() const;
    };

    bool AABBNode::is_leaf() const {
        return left == -1 && right == -1;
    }

    struct AABBTree {
        std::vector<AABBNode> nodes;

        AABBNode& insert(const BoundingBox& bb);
        AABBNode* get_root();
        AABBNode* get_last();
        void recalculate_upwards(int32_t from_idx);
    };

    AABBNode* AABBTree::get_root() {
        return &nodes[0];
    }

    AABBNode* AABBTree::get_last() {
        return &nodes[nodes.size() -1];
    }

    void AABBTree::recalculate_upwards(int32_t from_idx) {
        while (from_idx != -1) {
            AABBNode* from = &nodes[from_idx];
            
            if (from->left == -1 && from->right == -1) {
                from->bb = from->bb;
            } else if (from->left == -1) {
                from->bb = nodes[from->left].bb;
            } else if (from->right == -1) {
                from->bb = nodes[from->right].bb;
            } else {
                from->bb = join(nodes[from->left].bb, nodes[from->right].bb);
            }

            from_idx = from->parent;
        }
    }

    AABBNode& AABBTree::insert(const BoundingBox& bb) {
        int32_t insert_idx = nodes.size();
        nodes.push_back(AABBNode::empty());
        nodes[insert_idx].bb = bb;

        if (insert_idx > 0) {
            int32_t node_idx = 0;
            AABBNode* swap_node = &nodes[node_idx];

            while (!swap_node->is_leaf()) {
                const float left_volume = swap_node->left != -1 ? join(bb, nodes[swap_node->left].bb).surface_area() : std::numeric_limits<float>::max();
                const float right_volume = swap_node->right != -1 ? join(bb, nodes[swap_node->right].bb).surface_area() : std::numeric_limits<float>::max();

                node_idx = left_volume < right_volume ? swap_node->left : swap_node->right;
                swap_node = &nodes[node_idx];
            }
            
            nodes.push_back(AABBNode::empty());
            int32_t empty_idx = (int32_t) nodes.size() - 1;
            AABBNode* empty_node = &nodes[empty_idx];
            AABBNode* insert_node = &nodes[insert_idx];
            swap_node = &nodes[node_idx];

            // If `swap_node` is the root node
            if (node_idx == 0) {
                std::swap(*empty_node, *swap_node);
                std::swap(empty_idx, node_idx);

                swap_node = &nodes[node_idx];
                empty_node = &nodes[empty_idx];
                empty_node->left = node_idx;
            } else {
                if (nodes[nodes[node_idx].parent].left == node_idx) { nodes[nodes[node_idx].parent].left = empty_idx;
                } else { nodes[nodes[node_idx].parent].right = empty_idx; }
            }
            
            empty_node->parent = swap_node->parent; empty_node->left = node_idx; empty_node->right = insert_idx;
            swap_node->parent = empty_idx; swap_node->left = -1; swap_node->right = -1;
            insert_node->parent = empty_idx; insert_node->left = -1; insert_node->right = -1;
        }

        recalculate_upwards(insert_idx);
        return nodes[insert_idx];
    }

    // AABBNode& AABBTree::insert(const BoundingBox& bb) {
    //     AABBNode latest = AABBNode::empty();
    //     latest.bb = bb;
    //     int latest_idx = nodes.size();
    //     nodes.push_back(AABBNode::empty());

    //     AABBNode* node = get_root();
    //     if (node != get_last()) {
    //         std::cout << "The Inserted Node is snot the root node" << std::endl;
    //         int i = 0;
    //         int node_idx = 0;
    //         while (!node->is_leaf()) {
    //             const float left_volume = node->left != -1 ? join(bb, nodes[node->left].bb).surface_area() : std::numeric_limits<float>::max();
    //             const float right_volume = node->right != -1 ? join(bb, nodes[node->right].bb).surface_area() : std::numeric_limits<float>::max();
    //             // node = left_volume < right_volume ? node->left : node->right;
    //             if (left_volume < right_volume) {
    //                 node_idx = node->left;
    //             } else {
    //                 node_idx= node->right;
    //             }

    //             node = &nodes[node_idx];
    //             i += 1;
    //         }

    //         std::cout << "Iterations: " << i << std::endl;

    //         nodes.push_back(AABBNode::empty());
    //         AABBNode* hollow_node = get_last();
    //         node = &nodes[node_idx];

    //         hollow_node->parent = node->parent;
    //         hollow_node->left = node;
    //         hollow_node->right = latest;
 
    //         if (node == get_root()) {
    //             std::cout << "final leaf node was the root node" << std::endl;
    //             std::swap(*hollow_node, *node);
    //         } else {
    //             std::cout << "final leaf node was not the root node" << std::endl;
    //             if (node->parent->left == node) {
    //                 node->parent->left = hollow_node;
    //             } else {
    //                 node->parent->right = hollow_node;
    //             }
    //         }

    //         node->parent = hollow_node;
    //         latest->parent = hollow_node;
    //     }

    //     recalculate_upwards(latest);

    //     return *latest;
    // }
};


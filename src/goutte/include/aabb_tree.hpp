#pragma once

#include <boundingbox.hpp>

namespace gtt {
    /** Spatial Acceleration data structure that constructs a tree of Axis-Aligned Bounding Boxes.*/
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
        int32_t root = 0;
        std::vector<AABBNode> nodes;

        AABBNode& insert(const BoundingBox& bb);
        AABBNode* get_root();
        AABBNode* get_last();
        void recalculate_upwards(int32_t from_idx);
    };

    AABBNode* AABBTree::get_root() {
        return &nodes[root];
    }

    AABBNode* AABBTree::get_last() {
        return &nodes[nodes.size() -1];
    }

    void AABBTree::recalculate_upwards(int32_t from_idx) {
        while (from_idx != -1) {
            AABBNode& n = nodes[from_idx];

            if (!n.is_leaf()) {
                n.bb = join(nodes[n.left].bb, nodes[n.right].bb);
            }

            from_idx = n.parent;
        }
    }

    AABBNode& AABBTree::insert(const BoundingBox& bb) {
        int32_t insert_idx = nodes.size();
        nodes.push_back(AABBNode::empty());
        nodes[insert_idx].bb = bb;

        if (insert_idx != root) {
            int32_t node_idx = root;
            AABBNode* swap_node = &nodes[node_idx];

            while (!swap_node->is_leaf()) {
                const float left_sa= swap_node->left != -1 ? join(bb, nodes[swap_node->left].bb).surface_area() - nodes[swap_node->left].bb.surface_area() : std::numeric_limits<float>::max();
                const float right_sa = swap_node->right != -1 ? join(bb, nodes[swap_node->right].bb).surface_area() - nodes[swap_node->right].bb.surface_area() : std::numeric_limits<float>::max();

                node_idx = left_sa < right_sa ? swap_node->left : swap_node->right;
                swap_node = &nodes[node_idx];
            }
            
            nodes.push_back(AABBNode::empty());
            int32_t empty_idx = (int32_t) nodes.size() - 1;
            AABBNode* empty_node = &nodes[empty_idx];
            AABBNode* insert_node = &nodes[insert_idx];
            swap_node = &nodes[node_idx];

            // If `swap_node` is the root node
            if (node_idx == root) {
                root = empty_idx;
            } else {
                if (nodes[nodes[node_idx].parent].left == node_idx) { nodes[nodes[node_idx].parent].left = empty_idx; } 
                else { nodes[nodes[node_idx].parent].right = empty_idx; }
            }
            
            empty_node->parent = swap_node->parent; empty_node->left = node_idx; empty_node->right = insert_idx;
            swap_node->parent = empty_idx; swap_node->left = -1; swap_node->right = -1;
            insert_node->parent = empty_idx; insert_node->left = -1; insert_node->right = -1;
        }

        recalculate_upwards(nodes[insert_idx].parent);
        return nodes[insert_idx];
    }
};


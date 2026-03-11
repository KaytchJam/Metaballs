#pragma once

#include <boundingbox.hpp>

namespace gtt {
    /** Spatial Acceleration data structure that constructs a tree of Axis-Aligned Bounding Boxes.*/
    struct AABBTree;

    struct AABBNode {
        BoundingBox bb;
        void* data;
        int32_t parent;
    
        union {
            struct {
                int32_t left;
                int32_t right;
            };
            int32_t children[2];
        };

        static AABBNode empty() {
            return AABBNode { BoundingBox(), nullptr, -1, { -1, -1 } };
        }

        bool is_leaf() const;

        template <typename T>
        inline T* data_as() {
            return static_cast<T*>(data);
        }
    };

    bool AABBNode::is_leaf() const {
        return left == -1 && right == -1;
    }

    struct AABBTree {
        int32_t root = 0;
        std::vector<AABBNode> nodes;

        AABBNode& insert(void* data, const BoundingBox& bb);
        void recalculate_upwards(int32_t from_idx);

        inline AABBNode& left(AABBNode& n);
        inline AABBNode& right(AABBNode& n);
        inline AABBNode& parent(AABBNode& n);
    };

    inline AABBNode& AABBTree::left(AABBNode& n) {
        return nodes[n.left];
    }

    inline AABBNode& AABBTree::right(AABBNode& n) {
        return nodes[n.right];
    }

    inline AABBNode& AABBTree::parent(AABBNode& n) {
        return nodes[n.parent];
    }

    void AABBTree::recalculate_upwards(int32_t from_idx) {
        while (from_idx != -1) {
            AABBNode& n = nodes[from_idx];

            if (!n.is_leaf()) {
                n.bb = join(left(n).bb, right(n).bb);
            }

            from_idx = n.parent;
        }
    }

    AABBNode& AABBTree::insert(void* data, const BoundingBox& bb) {
        int32_t insert_idx = nodes.size();
        nodes.push_back(AABBNode::empty());
        nodes[insert_idx].bb = bb;
        nodes[insert_idx].data = data;

        if (insert_idx != root) {
            constexpr float INF = std::numeric_limits<float>::max();
            int32_t node_idx = root;
            AABBNode* swap_node = &nodes[node_idx];

            while (!swap_node->is_leaf()) {
                float left_sa = INF;
                float right_sa = INF;

                if (swap_node->left != -1) {
                    AABBNode& sn_left = left(*swap_node);
                    left_sa = join(bb, sn_left.bb).surface_area() - sn_left.bb.surface_area();
                }

                if (swap_node->right != -1) {
                    AABBNode& sn_right = right(*swap_node);
                    right_sa = join(bb, sn_right.bb).surface_area() - sn_right.bb.surface_area();
                }

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
                AABBNode& parent_node = parent(*swap_node);
                uint8_t cond = (uint8_t) parent_node.right == node_idx;
                parent_node.children[cond] = empty_idx;
            }
            
            empty_node->parent = swap_node->parent; empty_node->left = node_idx; empty_node->right = insert_idx;
            swap_node->parent = empty_idx; swap_node->left = -1; swap_node->right = -1;
            insert_node->parent = empty_idx; insert_node->left = -1; insert_node->right = -1;
        }

        recalculate_upwards(nodes[insert_idx].parent);
        return nodes[insert_idx];
    }
};


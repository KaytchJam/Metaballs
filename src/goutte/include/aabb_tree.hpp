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

        inline bool is_leaf() const;
        inline AABBNode& update_links(int32_t parent, int32_t left, int32_t right);

        template <typename T>
        inline T* data_as() {
            return static_cast<T*>(data);
        }
    };

    bool AABBNode::is_leaf() const {
        return left == -1 && right == -1;
    }

    AABBNode& AABBNode::update_links(int32_t parent, int32_t left, int32_t right) {
        this->parent = parent;
        this->left = left;
        this->right = right;
        return *this;
    }

    struct AABBTree {
        int32_t root = 0;
        std::vector<AABBNode> nodes;

        int32_t insert(void* data, const BoundingBox& bb);
        void recalculate_upwards(int32_t from_idx);

        inline AABBNode* left(AABBNode* n);
        inline AABBNode* right(AABBNode* n);
        inline AABBNode* parent(AABBNode* n);

        template <typename F>
        void all_overlaps(F func);

        size_t count_leaves() const;

        enum OverlapLeafState {
            FIRST_IS_LEAF = 0,
            SECOND_IS_LEAF = 1,
            BOTH_IS_LEAF = 2,
            NEITHER_IS_LEAF = 3,
            NO_OVERLAP = 4
        };

        OverlapLeafState get_overlap_leaf_state(const AABBNode& a, const AABBNode& b) const;
    };

    inline AABBNode* AABBTree::left(AABBNode* n) {
        return &nodes[n->left];
    }

    inline AABBNode* AABBTree::right(AABBNode* n) {
        return &nodes[n->right];
    }

    inline AABBNode* AABBTree::parent(AABBNode* n) {
        return &nodes[n->parent];
    }

    size_t AABBTree::count_leaves() const {
        size_t count = 0;
        for (const AABBNode& ab : nodes) {
            count += (size_t) ab.is_leaf();
        }
        return count;
    }

    /** Iteratively travel up the tree, starting from the `AABBNode` at index from_idx,
     * and make its bounding box the result of joining the bounding boxes of its left
     * child node and right child node. */
    void AABBTree::recalculate_upwards(int32_t from_idx) {
        while (from_idx != -1) {
            AABBNode* n = &nodes[from_idx];

            if (!n->is_leaf()) {
                n->bb = join(left(n)->bb, right(n)->bb);
            }

            from_idx = n->parent;
        }
    }

    /** Insert some data into the AABB Tree, where the data is associated with
     * `BoundingBox` bb. The index of the inserted data is returned. */
    int32_t AABBTree::insert(void* data, const BoundingBox& bb) {
        int32_t insert_idx = (int32_t) nodes.size();
        nodes.push_back(AABBNode::empty());
        nodes[insert_idx].bb = scale(bb, 1.5f);
        nodes[insert_idx].data = data;

        if (insert_idx != root) {
            constexpr float INF = std::numeric_limits<float>::max();
            int32_t node_idx = root;
            AABBNode* swap_node = &nodes[node_idx];

            while (!swap_node->is_leaf()) {
                float left_sa = INF;
                float right_sa = INF;

                if (swap_node->left != -1) {
                    AABBNode* sn_left = left(swap_node);
                    left_sa = join(bb, sn_left->bb).surface_area() - sn_left->bb.surface_area();
                }

                if (swap_node->right != -1) {
                    AABBNode* sn_right = right(swap_node);
                    right_sa = join(bb, sn_right->bb).surface_area() - sn_right->bb.surface_area();
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
                AABBNode* parent_node = parent(swap_node);
                const uint8_t cond = (uint8_t) parent_node->right == node_idx;
                parent_node->children[cond] = empty_idx;
            }
            
            empty_node->update_links(swap_node->parent, node_idx, insert_idx);
            swap_node->update_links(empty_idx, -1, -1);
            insert_node->update_links(empty_idx, -1, -1);
        }

        recalculate_upwards(nodes[insert_idx].parent);
        return insert_idx;
    }

    /** Helper function that takes in two `const AABBNode&` and returns the corresponding OverlapLeafState
     * of these two Nodes. */
    AABBTree::OverlapLeafState AABBTree::get_overlap_leaf_state(const AABBNode& a, const AABBNode& b) const {
        if (!overlap(a.bb, b.bb)) {
            return OverlapLeafState::NO_OVERLAP;
        }

        const bool a_is_leaf = a.is_leaf();
        const bool b_is_leaf = b.is_leaf();
        
        OverlapLeafState out_state = OverlapLeafState::NEITHER_IS_LEAF;
        if (a_is_leaf && b_is_leaf) {
            out_state = OverlapLeafState::BOTH_IS_LEAF;
        } else if (a_is_leaf) {
            out_state = OverlapLeafState::FIRST_IS_LEAF;
        } else if (b_is_leaf) {
            out_state = OverlapLeafState::SECOND_IS_LEAF;
        }

        return out_state;
    }

    template <typename F>
    void AABBTree::all_overlaps(F overlap_callback) {
        using IndexPair = std::pair<int32_t,int32_t>;
        std::vector<IndexPair> node_stack = {{root, root}};
        size_t stack_size = 1;

        void (*push_item)(std::vector<IndexPair>&,size_t&,int32_t,int32_t) = [](std::vector<IndexPair>& stack, size_t& ss, int32_t a, int32_t b) {
            if (ss >= stack.size()) { stack.push_back({a, b}); } 
            else { stack[ss] = {a, b}; }
            ss += 1;
        };

        while (stack_size > 0) {
            // pop
            stack_size -= 1;
            const IndexPair frame = node_stack[stack_size];
            AABBNode* A = &nodes[frame.first];
            AABBNode* B = &nodes[frame.second];

            switch(get_overlap_leaf_state(*A, *B)) {
                // We found an overlap
                case OverlapLeafState::BOTH_IS_LEAF:
                    if (frame.first != frame.second) { overlap_callback(frame.first, frame.second); }
                    break;

                // continue traversal along the second node, B
                case OverlapLeafState::FIRST_IS_LEAF:
                    push_item(node_stack, stack_size, frame.first, B->left);
                    push_item(node_stack, stack_size, frame.first, B->right); 
                    break;
                    
                // continue traversal along the first node, A
                case OverlapLeafState::SECOND_IS_LEAF:
                    push_item(node_stack, stack_size, A->left, frame.second);
                    push_item(node_stack, stack_size, A->right, frame.second);
                    break;

                // traverse down both A & B simultaneously
                case OverlapLeafState::NEITHER_IS_LEAF:
                    if (A != B) { push_item(node_stack, stack_size, A->right, B->left); }
                    push_item(node_stack, stack_size, A->left, B->left);
                    push_item(node_stack, stack_size, A->left, B->right);
                    push_item(node_stack, stack_size, A->right, B->right);
                    break;

                // also covers the NO OVERLAP case. We do nothing.
                default:
                    break;
            }
        }
    }
};


#pragma once

#include <boundingbox.hpp>
#include <dsa/indexstack.hpp>

namespace gtt {
    /** Spatial Acceleration data structure that constructs a tree of Axis-Aligned 
     * Bounding Boxes.*/
    struct AABBTree;

    struct AABBNode {
        BoundingBox bb;
        int32_t data_index;
        int32_t parent;
    
        union {
            struct {
                int32_t left;
                int32_t right;
            };
            int32_t children[2];
        };

        static AABBNode empty() {
            return AABBNode { BoundingBox(), -1, -1, { -1, -1 } };
        }

        inline bool is_leaf() const;
        inline AABBNode& update_links(int32_t parent, int32_t left, int32_t right);
    };

    struct AABBTree {
        int32_t root = 0;
        std::vector<AABBNode> nodes;

        /** Insert some data into the AABB Tree, where the data is associated with
        * `BoundingBox` bb. The index of the inserted data is returned. */
        int32_t insert(int32_t data_index, const BoundingBox& bb);

        int32_t find_swap_node(int32_t swap_idx, const BoundingBox& bb);

        /** Iteratively travel up the tree, starting from the `AABBNode` at index from_idx,
        * and make its bounding box the result of joining the bounding boxes of its left
        * child node and right child node. */
        void recalculate_upwards(int32_t from_idx);

        /** Count the number of leaves in this `AABBTree`. */
        size_t count_leaves() const;

        inline AABBNode* nuclear(AABBNode* n, int32_t AABBNode::* member);
        inline const AABBNode* nuclear(const AABBNode* n, int32_t AABBNode::* member) const;
        inline AABBNode* unuclear(AABBNode* n, int32_t AABBNode::* member);
        inline const AABBNode* unuclear(const AABBNode* n, int32_t AABBNode::* member) const;

        inline AABBNode* left(AABBNode* n);                         // Given a pointer to `node n`, return a pointer to the left child node of `n`, and nullptr if either (n == nullptr) or (n->left == -1).
        inline const AABBNode* left(const AABBNode* n) const;       // Given a pointer to `node n`, return a pointer to the left child node of `n`, and nullptr if either (n == nullptr) or (n->left == -1).
        inline AABBNode* uleft(AABBNode* n);                        // "unchecked" left. Return a pointer to `node n`'s left child.
        inline const AABBNode* uleft(const AABBNode* n) const;      // "unchecked" left. Return a pointer to `node n`'s left child.

        inline AABBNode* right(AABBNode* n);                        // Given a pointer to `node n`, return a pointer to the right child node of `n`, and nullptr if either (n == nullptr) or (n->right == -1).
        inline const AABBNode* right(const AABBNode* n) const;      // Given a pointer to `node n`, return a pointer to the right child node of `n`, and nullptr if either (n == nullptr) or (n->right == -1).
        inline AABBNode* uright(AABBNode* n);                       // "unchecked" right. Return a pointer to `node n`'s right child.
        inline const AABBNode* uright(const AABBNode* n) const;     // "unchecked" right. Return a pointer to `node n`'s right child.

        inline AABBNode* parent(AABBNode* n);                       // Given a pointer to `node n`, return a pointer to the parent node of `n`, and nullptr if either (n == nullptr) or (n->parent == -1).
        inline const AABBNode* parent(const AABBNode* n) const;     // Given a pointer to `node n`, return a pointer to the parent node of `n`, and nullptr if either (n == nullptr) or (n->parent == -1).
        inline AABBNode* uparent(AABBNode* n);                      // "unchecked" parent. Return a pointer to `node n`'s parent.
        inline const AABBNode* uparent(const AABBNode* n) const;    // "unchecked" parent. Return a pointer to `node n`'s parent.
     
        inline AABBNode* sibling(AABBNode* n);                      // Returns a pointer to the 'sibling' node of input node pointer `n`. `nullptr` returned if no such sibling exists.
        inline const AABBNode* sibling(const AABBNode* n) const;    // Returns a pointer to the 'sibling' node of input node pointer `n`. `nullptr` returned if no such sibling exists.

        template <typename F>
        void all_overlaps(F&& overlap_callback);

        enum OverlapLeafState {
            FIRST_IS_LEAF = 0,
            SECOND_IS_LEAF = 1,
            BOTH_IS_LEAF = 2,
            NEITHER_IS_LEAF = 3,
            NO_OVERLAP = 4
        };

        /** Helper function that takes in two `const AABBNode&` and returns the corresponding OverlapLeafState
        * of these two Nodes. */
        OverlapLeafState get_overlap_leaf_state(const AABBNode& a, const AABBNode& b) const;
    };

    template <typename F>
    void AABBTree::all_overlaps(F&& overlap_callback) {
        using IndexPair = std::pair<int32_t,int32_t>;
        
        dsa::IndexedStack<IndexPair> node_stack;
        node_stack.push({root, root});

        while (!node_stack.empty()) {
            // pop
            const IndexPair frame = node_stack.pop();
            AABBNode* A = &nodes[frame.first];
            AABBNode* B = &nodes[frame.second];

            switch(get_overlap_leaf_state(*A, *B)) {
                // We found an overlap
                case OverlapLeafState::BOTH_IS_LEAF:
                    if (frame.first != frame.second) { overlap_callback(frame.first, frame.second); }
                    break;

                // continue traversal along the second node, B
                case OverlapLeafState::FIRST_IS_LEAF:
                    node_stack.push({frame.first, B->left});
                    node_stack.push({frame.first, B->right}); 
                    break;
                    
                // continue traversal along the first node, A
                case OverlapLeafState::SECOND_IS_LEAF:
                    node_stack.push({A->left, frame.second});
                    node_stack.push({A->right, frame.second});
                    break;

                // traverse down both A & B simultaneously
                case OverlapLeafState::NEITHER_IS_LEAF:
                    if (A != B) { node_stack.push({A->right, B->left}); }
                    node_stack.push({A->left, B->left});
                    node_stack.push({A->left, B->right});
                    node_stack.push({A->right, B->right});
                    break;

                // also covers the NO OVERLAP case. We do nothing.
                default:
                    break;
            }
        }
    }
};


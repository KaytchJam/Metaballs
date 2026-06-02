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

    struct SwapRecord {
        int32_t former = -1;
        int32_t current = -1;
        SwapRecord(const int32_t prev = -1, const int32_t foll = -1) : former(prev), current(foll) {}
    };

    /** Stores the old and new indices of nodes that are moved around
     * during AABBTree::remove. `count` indicates the maximum number
     * of nodes moved. At most 3 nodes in the tree can be moved around
     * due to `remove`. Indices `[0, count)` are to be checked. */
    struct SwapBus {
        SwapRecord records[3];
        int32_t count = 0;
    };

    struct AABBTree {
        int32_t root = 0;
        std::vector<AABBNode> nodes;

        AABBTree() {}
        AABBTree(AABBTree&& other);

        /** Insert some data into the AABB Tree, where the data is associated with
        * `BoundingBox` bb. The index of the inserted data is returned. */
        int32_t insert(int32_t data_index, const BoundingBox& bb);
        SwapBus remove(const int32_t idx);

        // template <std::input_iterator Iter>
        // std::pair<int32_t,int32_t> insert_all(Iter low, Iter high);

        /** Iteratively travel up the tree, starting from the `AABBNode` at index from_idx,
        * and make its bounding box the result of joining the bounding boxes of its left
        * child node and right child node. */
        void recalculate_upwards(int32_t from_idx);

        /** Count the number of leaves in this `AABBTree`. */
        size_t count_leaves() const;

        /** Find all simultaneous  */
        template <typename F>
        void all_overlaps(F&& on_overlap);

        enum class OverlapLeafState {
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
    void AABBTree::all_overlaps(F&& on_overlap) {
        if (nodes.size() == 0) return;

        using IndexPair = std::pair<int32_t,int32_t>;
        dsa::IndexedStack<IndexPair> node_stack;
        node_stack.push({root, root});

        while (!node_stack.empty()) {
            // pop
            const IndexPair frame = node_stack.pop();
            AABBNode* const A = &nodes[frame.first];
            AABBNode* const B = &nodes[frame.second];

            switch(get_overlap_leaf_state(*A, *B)) {
                // We found an overlap
                case OverlapLeafState::BOTH_IS_LEAF:
                    if (frame.first != frame.second) { on_overlap(frame.first, frame.second); }
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


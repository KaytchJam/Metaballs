#include <aabb_tree.hpp>

namespace gtt {
    bool AABBNode::is_leaf() const {
        return left == -1 && right == -1;
    }
    
    AABBNode& AABBNode::update_links(int32_t parent, int32_t left, int32_t right) {
        this->parent = parent;
        this->left = left;
        this->right = right;
        return *this;
    }

    inline AABBNode* AABBTree::unuclear(AABBNode* n, int32_t AABBNode::* member) { return &nodes[n->*member]; }
    inline const AABBNode* AABBTree::unuclear(const AABBNode* n, int32_t AABBNode::* member) const { return &nodes[n->*member]; }
    inline AABBNode* AABBTree::nuclear(AABBNode* n, int32_t AABBNode::* member) { return n == nullptr || n->*member == -1 ? nullptr : &nodes[n->*member]; }
    inline const AABBNode* AABBTree::nuclear(const AABBNode* n, int32_t AABBNode::* member) const { return n == nullptr || n->*member == -1 ? nullptr : &nodes[n->*member]; }

    inline AABBNode* AABBTree::uleft(AABBNode* n) { return unuclear(n, &AABBNode::left); }
    inline const AABBNode* AABBTree::uleft(const AABBNode* n) const { return unuclear(n, &AABBNode::left); }
    inline AABBNode* AABBTree::left(AABBNode* n) { return nuclear(n, &AABBNode::left); }
    inline const AABBNode* AABBTree::left(const AABBNode* n) const { return nuclear(n, &AABBNode::left); }

    inline AABBNode* AABBTree::uright(AABBNode* n) { return unuclear(n, &AABBNode::right); }
    inline const AABBNode* AABBTree::uright(const AABBNode* n) const { return unuclear(n, &AABBNode::right); }
    inline AABBNode* AABBTree::right(AABBNode* n) { return nuclear(n, &AABBNode::right); }
    inline const AABBNode* AABBTree::right(const AABBNode* n) const { return nuclear(n, &AABBNode::right); }

    inline AABBNode* AABBTree::uparent(AABBNode* n) { return unuclear(n, &AABBNode::parent); }
    inline const AABBNode* AABBTree::uparent(const AABBNode* n) const { return unuclear(n, &AABBNode::parent); }
    inline AABBNode* AABBTree::parent(AABBNode* n) { return nuclear(n, &AABBNode::parent); }
    inline const AABBNode* AABBTree::parent(const AABBNode* n) const { return nuclear(n, &AABBNode::parent); }

    inline AABBNode* AABBTree::sibling(AABBNode* n) {
        if (n == nullptr || n->is_leaf()) { return  nullptr; }
        const AABBNode* p = uparent(n);
        return &nodes[p->children[1 - (uleft(p) == n ? p->left : p->right)]];
    }

    inline const AABBNode* AABBTree::sibling(const AABBNode* n) const {
        if (n == nullptr || n->is_leaf()) { return  nullptr; }
        const AABBNode* p = uparent(n);
        return &nodes[p->children[1 - (uleft(p) == n ? p->left : p->right)]];
    }

    size_t AABBTree::count_leaves() const {
        size_t count = 0;
        for (const AABBNode& ab : nodes) {
            count += (size_t) ab.is_leaf();
        }
        return count;
    }

    void AABBTree::recalculate_upwards(int32_t from_idx) {
        while (from_idx != -1) {
            AABBNode* n = &nodes[from_idx];

            if (!n->is_leaf()) {
                n->bb = join(uleft(n)->bb, uright(n)->bb);
            }

            from_idx = n->parent;
        }
    }

    int32_t AABBTree::insert(int32_t data_index, const BoundingBox& bb) {
        int32_t insert_idx = (int32_t) nodes.size();
        nodes.push_back(AABBNode::empty());
        nodes[insert_idx].bb = scale(bb, 1.1f);
        nodes[insert_idx].data_index = data_index;

        if (insert_idx != root) {
            constexpr float INF = std::numeric_limits<float>::max();
            int32_t node_idx = root;
            AABBNode* swap_node = &nodes[node_idx];

            while (!swap_node->is_leaf()) {
                float left_sa = INF;
                float right_sa = INF;

                if (swap_node->left != -1) {
                    const AABBNode* sn_left = uleft(swap_node);
                    left_sa = join(bb, sn_left->bb).surface_area() - sn_left->bb.surface_area();
                }

                if (swap_node->right != -1) {
                    const AABBNode* sn_right = uright(swap_node);
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
                AABBNode* parent_node = uparent(swap_node);
                parent_node->children[parent_node->right == node_idx] = empty_idx;
            }
            
            empty_node->update_links(swap_node->parent, node_idx, insert_idx);
            swap_node->update_links(empty_idx, -1, -1);
            insert_node->update_links(empty_idx, -1, -1);
        }

        recalculate_upwards(nodes[insert_idx].parent);
        return insert_idx;
    }

    AABBTree::OverlapLeafState AABBTree::get_overlap_leaf_state(const AABBNode& a, const AABBNode& b) const {
        if (!overlapping(a.bb, b.bb)) {
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
}
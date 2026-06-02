#include <aabb_tree.hpp>
#include <iostream>

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

    // inline AABBNode* get_node(AABBTree& tree, const int32_t idx) { return (0 <= idx && idx < (int32_t) tree.nodes.size()) ? &tree.nodes[idx] : nullptr; }
    // inline AABBNode* uget_node(AABBTree& tree, const int32_t idx) { return &tree.nodes[idx]; }

    inline AABBNode* unuclear(AABBTree& tree, AABBNode* n, int32_t AABBNode::* member) { return &tree.nodes[n->*member]; }
    inline const AABBNode* unuclear(const AABBTree& tree, const AABBNode* n, int32_t AABBNode::* member) { return &tree.nodes[n->*member]; }
    inline AABBNode* nuclear(AABBTree& tree, AABBNode* n, int32_t AABBNode::* member) { return n == nullptr || n->*member == -1 ? nullptr : &tree.nodes[n->*member]; }
    inline const AABBNode* nuclear(const AABBTree& tree, const AABBNode* n, int32_t AABBNode::* member) { return n == nullptr || n->*member == -1 ? nullptr : &tree.nodes[n->*member]; }

    inline AABBNode* uleft(AABBTree& tree, AABBNode* n) { return unuclear(tree, n, &AABBNode::left); }
    inline const AABBNode* uleft(const AABBTree& tree, const AABBNode* n) { return unuclear(tree, n, &AABBNode::left); }
    inline AABBNode* left(AABBTree& tree, AABBNode* n) { return nuclear(tree, n, &AABBNode::left); }
    inline const AABBNode* left(const AABBTree& tree, const AABBNode* n) { return nuclear(tree, n, &AABBNode::left); }

    inline AABBNode* uright(AABBTree& tree, AABBNode* n) { return unuclear(tree, n, &AABBNode::right); }
    inline const AABBNode* uright(const AABBTree& tree, const AABBNode* n) { return unuclear(tree, n, &AABBNode::right); }
    inline AABBNode* right(AABBTree& tree, AABBNode* n) { return nuclear(tree, n, &AABBNode::right); }
    inline const AABBNode* right(const AABBTree& tree, const AABBNode* n) { return nuclear(tree, n, &AABBNode::right); }

    inline AABBNode* uparent(AABBTree& tree, AABBNode* n) { return unuclear(tree, n, &AABBNode::parent); }
    inline const AABBNode* uparent(const AABBTree& tree, const AABBNode* n) { return unuclear(tree, n, &AABBNode::parent); }
    inline AABBNode* parent(AABBTree& tree, AABBNode* n) { return nuclear(tree, n, &AABBNode::parent); }
    inline const AABBNode* parent(const AABBTree& tree, const AABBNode* n) { return nuclear(tree, n, &AABBNode::parent); }

    inline AABBNode* sibling(AABBTree& tree, AABBNode* n) {
        if (n == nullptr) { return  nullptr; }
        const AABBNode* p = uparent(tree, n);
        return &tree.nodes[uleft(tree, p) == n ? p->right : p->left];
    }

    inline const AABBNode* sibling(const AABBTree& tree, const AABBNode* n) {
        if (n == nullptr) { return  nullptr; }
        const AABBNode* p = uparent(tree, n);
        return &tree.nodes[uleft(tree, p) == n ? p->right : p->left];
    }

    size_t AABBTree::count_leaves() const {
        size_t count = 0;
        for (const AABBNode& ab : nodes) {
            count += (size_t) ab.is_leaf();
        }
        return count;
    }

    void AABBTree::recalculate_upwards(int32_t from_idx) {
        AABBTree& self = *this;
        while (from_idx != -1) {
            AABBNode* n = &nodes[from_idx];

            if (!n->is_leaf()) {
                n->bb = join(uleft(self, n)->bb, uright(self, n)->bb);
            }

            from_idx = n->parent;
        }
    }

    int32_t AABBTree::insert(int32_t data_index, const BoundingBox& bb) {
        const int32_t insert_idx = (int32_t) nodes.size();
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
                    const AABBNode* sn_left = uleft(*this, swap_node);
                    left_sa = join(bb, sn_left->bb).surface_area() - sn_left->bb.surface_area();
                }

                if (swap_node->right != -1) {
                    const AABBNode* sn_right = uright(*this, swap_node);
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
                AABBNode* parent_node = uparent(*this, swap_node);
                parent_node->children[parent_node->right == node_idx] = empty_idx;
            }
            
            empty_node->update_links(swap_node->parent, node_idx, insert_idx);
            swap_node->update_links(empty_idx, -1, -1);
            insert_node->update_links(empty_idx, -1, -1);
        }

        recalculate_upwards(nodes[insert_idx].parent);
        return insert_idx;
    }

    SwapRecord swap_and_pop(AABBTree& tree, const int32_t remove_idx) {
        const int32_t end_idx = (int32_t) (tree.nodes.size() - 1);
        std::cout << "(SWAP AND POP) swapping " << remove_idx << " with " << end_idx << std::endl;
        // std::cout << "updating links" << std::endl;
        
        if (end_idx != remove_idx) {
            // std::cout << "updating links of the end index" << std::endl;
            AABBNode* const relocatee = &tree.nodes[end_idx];
            AABBNode* const relocatee_parent = parent(tree, relocatee);

            // update the relocatee's parent's link & children link (if not leaf node)
            if (relocatee_parent) {
                relocatee_parent->children[relocatee_parent->right == end_idx] = remove_idx;
            }

            if (!relocatee->is_leaf()) {
                tree.nodes[relocatee->left].parent = remove_idx;
                tree.nodes[relocatee->right].parent = remove_idx;
            }
        }

        std::swap(tree.nodes[remove_idx], tree.nodes[end_idx]);
        return SwapRecord(end_idx, remove_idx);
    }

    void print_tree(const AABBTree& tree) {
        std::printf("ROOT = %d\n", tree.root);
        int32_t idx = 0;
        for (const AABBNode& n : tree.nodes) {
            std::printf("%d = (D: %d, P: %d, L: %d, R: %d)\n", idx, n.data_index, n.parent, n.left, n.right);
            idx += 1;
        }
    }

    void add_record(SwapBus& bus, const int32_t previous, const int32_t current) {
        bus.records[bus.count] = SwapRecord(previous, current);
        bus.count += 1;
    }

    SwapRecord& top_record(SwapBus& bus) {
        return bus.records[bus.count - 1];
    }

    /** Returns true if the transfer is successful. False otherwise. */
    bool transfer(SwapRecord& old, const SwapRecord& recent) {
        if (old.current != -1 && old.current == recent.former) {
            old.current = recent.current;
            return true;
        }

        return false;
    }

    SwapBus AABBTree::remove(const int32_t remove_idx) {
        std::cout << "CHEKK 1" << std::endl;
        print_tree(*this);
        SwapBus bus;

        /** Only works on leaf nodes */
        if (!nodes[remove_idx].is_leaf()) {
            return bus;
        }
        
        // is the node we're removing the root? then we can just pop it
        if (remove_idx == root) {
            nodes.pop_back();
            return bus;
        }
        
        // first, swap sibling with parent
        std::cout << "sibling swap" << std::endl;
        int32_t old_sibling_idx = -1;
        {
            AABBNode* const cur = &nodes[remove_idx];
            AABBNode* const sib = sibling(*this, cur);
            AABBNode* const par = parent(*this, cur);
            
            std::cout << "set sibling idx" << std::endl;

            // Update links & store sibling's (previous) index
            sib->parent = par->parent;
            old_sibling_idx = par->children[1 - (par->right == remove_idx)];

            // Remove all the parent's links
            par->parent = -1;
            par->left = -1;
            par->right = -1;

            // If the sibling is not a leaf node, tell its children where its new index is
            if (!sib->is_leaf()) {
                nodes[sib->left].parent = cur->parent;
                nodes[sib->right].parent = cur->parent;
            }
            
            if (sib->is_leaf()) {
                add_record(bus, old_sibling_idx, cur->parent);
            }

            // Swap the sibling and the parent
            std::cout << "Swap time :)" << std::endl;
            std::swap(*sib, *par);
            std::cout << "POST SIBLING SWAP CHECK: " << old_sibling_idx << " and " << cur->parent << std::endl;
            print_tree(*this);

        }
        
        // swap & pop remove_idx 
        std::cout << "swap and pop 1" << std::endl;
        
        SwapRecord nouveau = swap_and_pop(*this, remove_idx);
        nodes.pop_back();

        // If no transfer & the swap node wasn't the new sibling idx & end_idx != remove_idx & swap node != old sibling idx (since we'll delete it anyways)...
        if (!transfer(bus.records[0], nouveau) && remove_idx != nouveau.former && old_sibling_idx != nouveau.former) {
            std::printf("NO TRANSFER BETWEEN (%d -> %d) and (%d -> %d)\n", bus.records[0].former, bus.records[0].current, nouveau.former, nouveau.current);
            add_record(bus, nouveau.former, nouveau.current);
        }

        std::cout << "FIRST SWAP & POP CHECK (CURRENT SIZE =" << nodes.size() << ")" << std::endl;
        print_tree(*this);

        std::cout << "SIBLING IDX: " << old_sibling_idx << std::endl;
        if (old_sibling_idx == nouveau.former) {
            old_sibling_idx = nouveau.current;
        }

        std::cout << "swap and pop 2" << std::endl;
        nouveau = swap_and_pop(*this, old_sibling_idx);
        nodes.pop_back();
        if (!transfer(bus.records[0], nouveau) && !transfer(bus.records[1], nouveau) && old_sibling_idx != nouveau.former) {
            add_record(bus, nouveau.former, nouveau.current);
        }


        std::cout << "EXIT CHECK" << std::endl;
        print_tree(*this);

        return bus;
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
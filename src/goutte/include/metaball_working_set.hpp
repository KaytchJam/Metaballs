#pragma once

#include <fieldrange.hpp>
#include <metaball_traits.hpp>
#include <aabb_tree.hpp>
#include <isosurface.hpp>

#include <dsa/wrappers.hpp>
#include <dsa/unionfind.hpp>

#include <span>
#include <sstream>
#include <iostream>

namespace gtt {
    template <typename Range>
    struct MetaballRenderGroup {
        FieldRange fr;
        Range ball_indices;
    };

    std::string to_string(const BoundingBox& bb) {
        std::stringstream ss;
        ss << "[MAX: (" << bb.max_point.x << "," << bb.max_point.y << "," << bb.max_point.z << ")" 
            << ", MIN: (" << bb.min_point.x << "," << bb.min_point.y << "," << bb.min_point.z << ")]";
        return ss.str();
    }

    FieldRange bbox_to_field(const IsoSurface& surface, const BoundingBox& bb) {
        const int32_t max_index = surface.shape()[0] - 1;
        const gtt::lalg::ivec3 start = clamp(lalg::ivec3(floor(surface.position_to_index(bb.min_point))), 0, max_index);
        const gtt::lalg::ivec3 end = clamp(lalg::ivec3(ceil(surface.position_to_index(bb.max_point))), 0, max_index);
        return FieldRange({start.x, end.x, start.y, end.y, start.z, end.z});
    };

    /** Container for a set of Metaballs in the Metaball Engine. Has specializations
     * such that it optimizes for when the metaball type has boudning boxes or not. */
    template <typename M, bool B = VALID_BOUNDED_METABALL(M)>
    struct MetaballWorkingSet {
        using GroupRangeType = IntRange;
        using RenderGroup = MetaballRenderGroup<GroupRangeType>;
        using MetaballRenderGroupRange = dsa::wrap::PlaceboWrapper<RenderGroup>;

        static constexpr bool has_bounding_box = false;

        MetaballWorkingSet(IsoSurface& s);
        ~MetaballWorkingSet();

        size_t add_metaball(M&& m) = 0;
        M& get_metaball(const size_t i);
        const M& get_metaball(const size_t i) const;
        MetaballRenderGroupRange groups() &;
    };
    
    /** Specialization for types `M` that don't satisfy `HasBoundingBox<M>`. */
    template <typename M>
    struct MetaballWorkingSet<M, false> {
        std::vector<dsa::wrap::PlaceboWrapper<M>> balls;
        const IsoSurface& surface;
        
        static constexpr bool has_bounding_box = false;

        using BallContainer = std::vector<dsa::wrap::PlaceboWrapper<M>>;
        using GroupRangeType = IntRange;
        using RenderGroup = MetaballRenderGroup<GroupRangeType>;
        using MetaballRenderGroupRange = dsa::wrap::PlaceboWrapper<RenderGroup>;

        MetaballWorkingSet(const IsoSurface& s) 
            : balls(), surface(s) {}

        MetaballRenderGroupRange groups() const & { 
            return MetaballRenderGroupRange(
                RenderGroup(
                    FieldRange(0, surface.shape()[0] - 1), 
                    IntRange(0, (int32_t) balls.size())
                )
            );
        }

        size_t add_metaball(M&& m) {
            size_t index = balls.size();
            balls.emplace_back(std::move(m));
            return index;
        }

        /** Get the metaball in this Metaball Engine at index i */
        M& get_metaball(const size_t i) {
            return *balls[i];
        }

        const M& get_metaball(const size_t i) const {
            return *balls[i];
        }
    };

    /** Specialization for types `M` that satisfy `HasBoundingBox<M>`. */
    template <typename M>
    struct MetaballWorkingSet<M, true> {
        std::vector<dsa::wrap::IndexWrapper<M>> balls;
        const IsoSurface& surface;

        AABBTree tree;
        dsa::UnionFind joiner;
        dsa::UnionFindCollector collector;
        bool regenerate_groups = true;

        static constexpr bool has_bounding_box = true;

        using BallContainer = std::vector<dsa::wrap::IndexWrapper<M>>;
        using Accessor = dsa::UnionFindCollector::Accessor;
        using GroupRangeType = std::span<const int32_t>;
        using RenderGroup = MetaballRenderGroup<GroupRangeType>;

        struct AccessorPlus {
            const Accessor& accessor;
            const BallContainer& balls;
            const IsoSurface& surface;
            const Accessor* operator->() const { return &accessor; }
        };

        MetaballWorkingSet(const IsoSurface& s) 
            : balls(), tree(), surface(s), joiner(0), collector() {
        }

        /** Effectively a rewrite of ComponentRangeIterator that accumulates / folds
         * by BoundingBox as it progresses. */
        struct MetaballRenderGroupRangeIterator {
            private:
                int32_t cur_index = 0;
                int32_t component_start_index = 0;
                BoundingBox accumulator = BoundingBox::empty();
                
                const AccessorPlus& accessor;

                /** Checks if two nodes are in the same component */
                inline bool component_equals(const int32_t a, const int32_t b) const {
                    return accessor->component_of(a) == accessor->component_of(b);
                }

                MetaballRenderGroupRangeIterator& advance() {
                    while (cur_index < accessor->size() && component_equals(cur_index, component_start_index)) {
                        accumulator = join(accumulator, accessor.balls[accessor.accessor.sorted_mappings[cur_index]]->get_bounding_box());
                        cur_index += 1;
                    }
                    return *this;
                }

            public:
                MetaballRenderGroupRangeIterator(const AccessorPlus& a, int32_t start)
                    : accessor(a), cur_index(start) {
                    advance();
                }

                MetaballRenderGroupRangeIterator(const AccessorPlus& a, int32_t start, int32_t component_start)
                    : accessor(a), cur_index(start), component_start_index(component_start) {}

                using value_type = RenderGroup;
                using reference = void;
                using pointer = void;
                using difference_type = std::ptrdiff_t;
                using iterator_category = std::forward_iterator_tag;
                
                value_type operator*() const  {
                    std::cout << "group : " << accessor->component_of(component_start_index) << ", start = " << component_start_index << " , end = " << cur_index <<  std::endl;
                    return value_type(
                        bbox_to_field(accessor.surface, accumulator),
                        std::span(accessor.accessor.sorted_mappings.data() + component_start_index, (size_t) cur_index - component_start_index )
                    );
                }

                MetaballRenderGroupRangeIterator& operator++() {
                    component_start_index = cur_index;
                    accumulator = BoundingBox::empty();
                    return advance();
                }

                MetaballRenderGroupRangeIterator operator++(int) {
                    MetaballRenderGroupRangeIterator dupe = MetaballRenderGroupRangeIterator(accessor, cur_index, component_start_index);
                    ++(*this);
                    return dupe;
                }

                bool operator==(const MetaballRenderGroupRangeIterator& other) const {
                    return component_start_index == other.component_start_index && cur_index == other.cur_index;
                }

                bool operator!=(const MetaballRenderGroupRangeIterator& other) const {
                    return !(*this == other);
                }
        };

        /** Range */
        struct MetaballRenderGroupRange {
            const AccessorPlus accessor;
            MetaballRenderGroupRange(AccessorPlus&& a) : accessor(a) {}
            using iterator = MetaballRenderGroupRangeIterator;
            iterator begin() const { return iterator(accessor, 0); }
            iterator end() const { return iterator(accessor, (int32_t) accessor->size(), (int32_t) accessor->size()); }
        };

        size_t add_metaball(M&& m) {
            size_t index = balls.size();
            int32_t node_index = tree.insert((int32_t) index, m.get_bounding_box());

            balls.emplace_back(std::move(m), node_index);
            tree.nodes[node_index].data_index = (int32_t) index;
            joiner.add_vertex();

            regenerate_groups = true;
            return index;
        }

        MetaballRenderGroupRange groups() & {
            if (regenerate_groups) {
                std::cout << "Re-fitting UnionFindCollector to UnionFind" << std::endl;

                BallContainer& balls_local = balls;
                gtt::AABBTree& tree_local = tree;
                gtt::dsa::UnionFind& joiner_local = joiner;

                tree_local.all_overlaps([&balls_local, &tree_local, &joiner_local](const int32_t a, const int32_t b) {
                    const int32_t mball_a = tree_local.nodes[a].data_index;
                    const int32_t mball_b = tree_local.nodes[b].data_index;

                    // the AABB Bounding Boxes overlap, but do the ACTUAL bounds overlap themselves?
                    if (overlapping(balls_local[mball_a]->get_bounding_box(), balls_local[mball_b]->get_bounding_box())) {
                        joiner_local.unite(mball_a, mball_b);
                    }
                });
    
                collector.fit(joiner);
                regenerate_groups = false;
            }

            return MetaballRenderGroupRange(
                AccessorPlus(collector.get_accessor(), balls, surface)
            );
        }

        /** Get the metaball in this Metaball Engine at integer index i. */
        M& get_metaball(const size_t i) {
            return *balls[i];
        }

        /** Get the metaball in this Metaball Engine at integer index i. */
        const M& get_metaball(const size_t i) const {
            return *balls[i];
        }
    };
}
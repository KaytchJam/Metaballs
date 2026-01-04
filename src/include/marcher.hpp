#include <fieldrange.hpp>
#include <isosurface.hpp>

namespace mbl {
    class MarchingCubeRange {
        private:
            IsoPoint* m_data;
            IndexCompactor reshaper;
            FieldRange field;
        public:
            MarchingCubeRange(IsoSurface& surface);
            ~MarchingCubeRange();

            /** Type returned by the MarchingCubeIterator. A 'view' of the underlying IsoSurface that
             * can be iterated over. */
            struct CubeView {
                MarchingCubeRange& parent;
                FieldRange fr;

                struct CubeViewIterator {
                    CubeView* view;
                    FieldRange::iterator it;

                    using value_type = IsoPoint;
                    using reference = IsoPoint&;
                    using pointer = IsoPoint*;
                    using iterator_category = std::input_iterator_tag;

                    reference operator*();
                    pointer operator->();

                    CubeViewIterator& operator++();
                    CubeViewIterator operator++(int);

                    bool operator==(const CubeViewIterator& other) const;
                    bool operator!=(const CubeViewIterator& other) const;
                };

                using iterator = CubeViewIterator;

                iterator begin() {
                    return iterator{ this, fr.begin() };
                }

                iterator end() {
                    return iterator{ this, fr.end() };
                }

                IsoPoint& at(int x, int y, int z) {
                    const IndexDim l = fr.low();
                    return parent.m_data[parent.reshaper.flatten(l.x + x, l.y + y, l.z + z)];
                }

                IsoPoint& at(int i) {
                    IndexDim idx(i % 2, i / 2, i / 4);
                    return at(idx.x, idx.y, idx.z);
                }
            };

            struct MarchingCubeIterator {
                private:
                    MarchingCubeRange* parent;
                    FieldRange m_window;
                    FieldRange::iterator cube_iterator;
                public:
                    MarchingCubeIterator(MarchingCubeRange& from, FieldRange::iterator iter);
                    ~MarchingCubeIterator();

                    using value_type = CubeView;
                    using reference = value_type;
                    using pointer = value_type;
                    using iterator_category = std::input_iterator_tag;

                    reference operator*();
                    MarchingCubeIterator& operator++();
                    MarchingCubeIterator operator++(int);

                    bool operator==(const MarchingCubeIterator& other) const;
                    bool operator!=(const MarchingCubeIterator& other) const;
            };
        
            using iterator = MarchingCubeIterator;

            iterator begin() {
                return iterator(*this, field.begin());
            }

            iterator end() {
                return iterator(*this, field.end());
            }
    };

    using CubeView = MarchingCubeRange::CubeView;
}
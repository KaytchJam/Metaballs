#include <marcher.hpp>

using namespace mbl;
using CubeView = MarchingCubeRange::CubeView;
using CubeViewIterator = CubeView::CubeViewIterator;
using MarchingCubeIterator = MarchingCubeRange::MarchingCubeIterator;

MarchingCubeRange::MarchingCubeRange(IsoSurface& surface) : m_data(surface.data()), reshaper(surface.shape()[0]), field(0, surface.shape()[0] - 1) {}

MarchingCubeRange::~MarchingCubeRange() {}

CubeViewIterator::reference CubeViewIterator::operator*() {
    const IndexDim indices_at = *it;
    return view->parent.m_data[view->parent.reshaper.flatten(indices_at[0], indices_at[1], indices_at[2])];
}

CubeViewIterator::pointer CubeViewIterator::operator->() {
    return &(*(*this));
}

CubeViewIterator& CubeViewIterator::operator++() {
    ++it;
    return *this;
}

CubeViewIterator CubeViewIterator::operator++(int) {
    CubeViewIterator prior = *this;
    ++(*this);
    return prior;
}

bool CubeViewIterator::operator==(const CubeViewIterator& other) const {
    return view->parent.m_data == other.view->parent.m_data && it == other.it;
}

bool CubeViewIterator::operator!=(const CubeViewIterator& other) const {
    return !((*this) == other);
}

FieldRange cube_range_from(const IndexDim& idx) {
    return FieldRange({idx.x, idx.x + 2, idx.y, idx.y + 2, idx.z, idx.z + 2});
}

MarchingCubeIterator::MarchingCubeIterator(MarchingCubeRange& from, FieldRange::iterator it) : parent(&from), m_window(cube_range_from(*it)), cube_iterator(it) {}

MarchingCubeIterator::~MarchingCubeIterator() {}

MarchingCubeIterator::reference MarchingCubeIterator::operator*() {
    return CubeView {
        *parent,
        this->m_window
    };
}

MarchingCubeIterator& MarchingCubeIterator::operator++() {
    ++cube_iterator;
    m_window = cube_range_from(*cube_iterator);
    return *this;
}

MarchingCubeIterator MarchingCubeIterator::operator++(int) {
    MarchingCubeIterator prior = (*this);
    ++(*this);
    return prior;
}

bool MarchingCubeIterator::operator==(const MarchingCubeIterator& other) const {
    return parent == other.parent && cube_iterator == other.cube_iterator;
}

bool MarchingCubeIterator::operator!=(const MarchingCubeIterator& other) const {
    return !(*this == other);
}

#include <common/lalg.hpp>
#include <isosurface.hpp>

int main() {
    const mbl::lalg::vec3 center = mbl::lalg::vec3(0.f);
    mbl::IsoSurface surface = mbl::IsoSurface::construct(mbl::lalg::vec3(0), 2, 4);

    mbl::lalg::vec3 q1 = mbl::lalg::vec3(-0.5f, 0, -1.1f);
    mbl::lalg::vec3 q2 = mbl::lalg::vec3(0.75f);

    mbl::lalg::ivec3 p1 = floor(surface.position_to_index(q1));
    mbl::lalg::ivec3 p2 = ceil(surface.position_to_index(q2));

    std::cout << "(" << q1.x << "," << q1.y << "," << q1.z << ") -> (" << p1.x << "," << p1.y << "," << p1.z << ")" << std::endl;
    std::cout << "(" << q2.x << "," << q2.y << "," << q2.z << ") -> (" << p2.x << "," << p2.y << "," << p2.z << ")" << std::endl;

    return EXIT_SUCCESS;
}
#include <common/lalg.hpp>
#include <isosurface.hpp>

int main() {
    const gtt::lalg::vec3 center = gtt::lalg::vec3(0.f);
    gtt::IsoSurface surface = gtt::IsoSurface::construct(gtt::lalg::vec3(0), 2, 4);

    gtt::lalg::vec3 q1 = gtt::lalg::vec3(-0.5f, 0, -1.1f);
    gtt::lalg::vec3 q2 = gtt::lalg::vec3(0.75f);

    gtt::lalg::ivec3 p1 = floor(surface.position_to_index(q1));
    gtt::lalg::ivec3 p2 = ceil(surface.position_to_index(q2));

    std::cout << "(" << q1.x << "," << q1.y << "," << q1.z << ") -> (" << p1.x << "," << p1.y << "," << p1.z << ")" << std::endl;
    std::cout << "(" << q2.x << "," << q2.y << "," << q2.z << ") -> (" << p2.x << "," << p2.y << "," << p2.z << ")" << std::endl;

    return EXIT_SUCCESS;
}
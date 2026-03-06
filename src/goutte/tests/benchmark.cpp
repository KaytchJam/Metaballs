#include <iostream>
#include <engine.hpp>
#include <metaball_presets.hpp>

#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtc/random.hpp"

int main() {
    const gtt::lalg::vec3 center = gtt::lalg::vec3(0.f);
    const float side_length = 10.f;
    const int32_t resolution = 30;
    const float iso_value = 1.f;
    const int32_t num_metaballs = 10;

    gtt::MetaballEngine<gtt::Metaball<gtt::presets::KineticBlob>> engine(center, side_length, resolution, iso_value);
    for (int i = 0; i < num_metaballs; i++) {
        gtt::lalg::vec3 position = gtt::lalg::vec3::from(glm::linearRand(glm::vec3(-5.f), glm::vec3(5.f)));
        gtt::lalg::vec3 velocity = gtt::lalg::vec3::from(glm::sphericalRand(1.f));
        engine.add_metaball(gtt::Metaball(gtt::presets::KineticBlob(position, velocity)));
    }

    gtt::common::graphics::MeshData md = engine.construct_mesh();

    /** How long does it take to render... 60 frames? */

    return EXIT_SUCCESS;
}
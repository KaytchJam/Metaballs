#include <iostream>
#include <chrono>

#include <engine.hpp>
#include <metaball_presets.hpp>

#include "../../dependencies/glm/glm.hpp"
#include "../../dependencies/glm/gtc/random.hpp"

constexpr int NUM_FRAMES = 240;

// { POSITION, VELOCITY } pairs 
constexpr gtt::lalg::vec3 attrs[20] = {
    {-1.26043f,-1.284f,4.33654f}, {0.82353f,-0.438463f,-0.35993f},
    {-2.99506f,-1.39082f,-4.79224f},{0.682832f,0.416807f,0.60001f},
    {-0.90648f,-3.15471f,4.31167f},{-0.408398f,0.198385f,-0.890985f},
    {-1.46316f,1.29168f,3.14258f},{0.182731f,0.534051f,0.825469f},
    {-2.86599f,-0.51078f,-3.30181f},{-0.661888f,0.153545f,0.733709f},
    {2.23479f,1.87668f,2.8832f},{0.539809f,-0.671457f,0.507693f},
    {0.109578f,-2.45794f,-2.35305f},{-0.952111f,0.294792f,-0.0811341f},
    {3.28063f,-3.15359f,3.91841f},{-0.746782f,0.664799f,-0.0189268f},
    {4.2237f,-1.41228f,-0.108713f},{0.078922f,0.239379f,-0.967713f},
    {-0.97312f,1.20523f,3.28367f},{-0.92494f,0.0265955f,-0.379183f}
};

int main() {
    const gtt::lalg::vec3 center = gtt::lalg::vec3(0.f);
    const float side_length = 10.f;
    const float iso_value = 1.f;
    const int32_t resolution = 30;
    const int32_t num_metaballs = 10;

    gtt::MetaballEngine<gtt::Metaball<gtt::presets::KineticBlob>> engine(center, side_length, resolution, iso_value);
    for (int i = 0; i < num_metaballs; i++) {
        const int true_i = i*2;
        const gtt::lalg::vec3& position = attrs[true_i];
        const gtt::lalg::vec3& velocity = attrs[true_i+1];
        engine.add_metaball(gtt::Metaball(gtt::presets::KineticBlob(position, velocity)));
    }

    
    /** Store the time it took each frame */
    float time[NUM_FRAMES];
    gtt::common::graphics::MeshData md;

    auto prev_time = std::chrono::steady_clock::now();

    /** How long does it take to render... NUM_FRAMES frames? */
    float lastFrame = 0.f;
    for (int i = 0; i < NUM_FRAMES; i++) {
        
        auto curr_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = curr_time - prev_time;
        prev_time = curr_time;
        
        float dt = (float) deltaTime.count();
        time[i] = dt;

        for (int i = 0; i < num_metaballs; i++) {
            gtt::presets::KineticBlob& kb = engine.get_metaball((size_t) i).unwrap();
            gtt::lalg::vec3& kb_pos = kb.update(dt);

            for (int i = 0; i < 3; i++) {
                if (kb_pos[i] < -5.f + 1.0f) {
                    kb_pos[i] = -4.0f;
                    kb.m_velocity[i] *= -1;
                } else if (kb_pos[i] > 5.f - 1.0f) {
                    kb_pos[i] = 4.0f;
                    kb.m_velocity[i] *= -1;
                }
            }
        }

        // Make it dirty so we can "redraw", then construct mesh
        md = engine.make_dirty().construct_mesh();
    }

    std::cout << "delta_times: list[float] = [";
    for (int i = 0; i < NUM_FRAMES - 1; i++) {
        std::cout << time[i] << ",";
    }

    if (NUM_FRAMES > 1) {
        std::cout << time[NUM_FRAMES - 1];
    }

    std::cout << "];" << std::endl;

    return EXIT_SUCCESS;
}
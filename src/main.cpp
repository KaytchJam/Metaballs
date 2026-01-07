
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <array>
#include <vector>

#include "dependencies/glm/ext/matrix_clip_space.hpp"
#include "dependencies/glm/ext/matrix_transform.hpp"
#include "dependencies/glm/gtc/type_ptr.hpp"

#include "dependencies/glfw-3.4/deps/glad/gl.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"

#include "convenience.hpp"
#include "Shader.hpp"
#include "Metaball.hpp"

struct MeshView {
    const std::vector<Vertex>* vertex_data;
    const std::vector<GLuint>* indices;
};

template <size_t GRID_SIZE>
void re_render_metaball_engine(MetaballEngine<GRID_SIZE>& me, MeshView& mview, GLuint& vbo, GLuint& ebo) {
    me.refresh();
    mview.vertex_data = &me.get_vertices();
    mview.indices = &me.get_indices();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mview.vertex_data->size() * sizeof(Vertex), mview.vertex_data->data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mview.indices->size() * sizeof(GLuint), mview.indices->data(), GL_DYNAMIC_DRAW);
}

template<size_t SCENES, size_t GRID_SIZE>
struct MetaballSceneViewer {
    size_t scene_at = 0;
    std::array<MetaballEngine<GRID_SIZE>, SCENES> scenes;
    std::array<glm::vec3, SCENES> mball_color;

    MetaballEngine<GRID_SIZE>& get_current_scene() {
        return this->scenes[this->scene_at];
    }

    const glm::vec3& get_current_color() {
        return this->mball_color[this->scene_at];
    }

    MetaballSceneViewer<SCENES, GRID_SIZE>& shift_left() {
        if (this->scene_at == 0) {
            this->scene_at = SCENES - 1;
        } else {
            this->scene_at -= 1;
        }

        return *this;
    }

    MetaballSceneViewer<SCENES, GRID_SIZE>& shift_right() {
        if (this->scene_at == SCENES - 1) {
            this->scene_at = 0;
        } else {
            this->scene_at += 1;
        }

        return *this;
    }
};

template<size_t SCENES, size_t GRID_SIZE>
using MSV = MetaballSceneViewer<SCENES, GRID_SIZE>;

constexpr size_t NUM_SCENES = 10;

template <size_t GRID_SIZE>
MSV<NUM_SCENES, GRID_SIZE> setup_scenes() {
    MSV<NUM_SCENES, GRID_SIZE> mball_scenes;
    
    mball_scenes.mball_color[0] = glm::vec3(0.f, 0.f, 1.f);
    MetaballEngine<GRID_SIZE> * m = &mball_scenes.scenes[0];
    m->add_metaball(glm::vec3(0.f)); // THE SIMPLE SCENE
    
    mball_scenes.mball_color[1] = glm::vec3(0.f, 1.f, 0.f);
    m = &mball_scenes.scenes[1];
    m->add_metaball(glm::vec3(0.f), tune_blob(2.f, 5.f, 1.f));
    m->add_metaball(glm::vec3(1.7f), tune_blob(8.f, 2.f, 2.f));
    m->add_metaball(glm::vec3(0.0, 2.0, 0.9f), tune_blob(10.f, 1.5f, 2.f));
    
    mball_scenes.mball_color[2] = glm::vec3(1.f, 0.f, 0.f);
    m = &mball_scenes.scenes[2];
    m->add_metaball(glm::vec3(0.f), tune_cube());
    
    mball_scenes.mball_color[3] = glm::vec3(1.f, 0.f, 1.f);
    m = &mball_scenes.scenes[3];
    m->add_metaball(glm::vec3(1.2f), tune_cube());
    m->add_metaball(glm::vec3(1.9f), tune_blob(1.f, 2.f, 3.f));
    
    mball_scenes.mball_color[4] = glm::vec3(0.8f, 0.8f, 0.8f);
    m = &mball_scenes.scenes[4];
    m->add_metaball(glm::vec3(0.0f), tune_gyroid());
    
    mball_scenes.mball_color[5] = glm::vec3(0.8f, 0.1f, 0.6f);
    m = &mball_scenes.scenes[5];
    m->add_metaball(glm::vec3(0.0f), tune_blob());
    m->add_metaball(glm::vec3(0.4f, 0.5f, 0.1f), tune_cube());
    m->add_metaball(glm::vec3(-0.3f, 2.1f, 1.4f), tune_cube());
    
    mball_scenes.mball_color[6] = glm::vec3(0.2, 0.9, 0.8);
    m = &mball_scenes.scenes[6];
    m->add_metaball(glm::vec3(0.0f), tune_cross(0.05f, 0.05f, 0.f));
    m->add_metaball(glm::vec3(0.0f), tune_plane(0.f, 1.f, 0.f, 0.f));
    
    mball_scenes.mball_color[7] = glm::vec3(1.0, 0.9, 0.2);
    m = &mball_scenes.scenes[7];
    m->add_metaball(glm::vec3(0.0f, 0.5f, 0.f), tune_blob());
    m->add_metaball(glm::vec3(0.0f), tune_plane(0.f, 1.f, 0.f, 1.f));
    
    mball_scenes.mball_color[8] = glm::vec3(0.0, 0.4, 0.5);
    m = &mball_scenes.scenes[8];
    m->add_metaball(glm::vec3(0.0f), tune_gyroid(0.f, 0.f, 1.f));
    m->add_metaball(glm::vec3(0.5, 0.f, 0.f), tune_paraboliod());
    m->add_metaball(glm::vec3(0.f), tune_plane(0.f, 1.f, 0.f));
    
    mball_scenes.mball_color[9] = glm::vec3(0.2, 0.2, 0.2);
    m = &mball_scenes.scenes[9];
    m->add_metaball(glm::vec3(0.0f), tune_star(2.f));
    
    return mball_scenes;
}

template <size_t S, size_t GRID_SIZE>
void check_keypress(GLFWwindow* w, MSV<S, GRID_SIZE>& scenes) {
    if (glfwGetKey(w, GLFW_KEY_LEFT) == GLFW_PRESS) {
        scenes.shift_left();
    } else if (glfwGetKey(w, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        scenes.shift_right();
    }
}

int metaball_scenes() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes Example").open();
    
    constexpr size_t GS = 60;
    MSV<NUM_SCENES, GS> scenes = setup_scenes<GS>();
    MetaballEngine<GS>* me = &scenes.get_current_scene();
    size_t prev_scene = NUM_SCENES + 1;
    
    MeshView mv = MeshView { &me->get_vertices(), &me->get_indices() };
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mv.vertex_data->size() * sizeof(Vertex), mv.vertex_data->data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mv.indices->size() * sizeof(GLuint), mv.indices->data(), GL_STATIC_DRAW);

    Shader s = Shader::from_file(
        "./src/shaders/vertex/vertex_lighting.vert",
        "./src/shaders/fragment/vertex_lighting.frag"
    ).value();

    s.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
    });

    s.add_uniform("color", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(1.0f, 0.f, 0.f)[0]);
    });

    GLuint program = s.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");
    const GLint vnorm_location = glGetAttribLocation(program, "pNorm");

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(vnorm_location);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.f),
        (float) SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.f
    );

    float lastFrame = 0.0f;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); uncomment to see all the individual vertices

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(win, deltaTime);
        check_keypress(win, scenes);

        if (scenes.scene_at != prev_scene) {
            prev_scene = scenes.scene_at;
            re_render_metaball_engine(scenes.get_current_scene(), mv, vbo, ebo);

            s.add_uniform("color", [&scenes](GLuint pgrm, GLint loc) {
                glUniform3fv(loc, 1, &scenes.get_current_color()[0]);
            });
        }

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view * model;

        s.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });
        
        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei) mv.indices->size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}

int animation_test() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "MC Animation Test").open();

    constexpr size_t SIZE = 30;
    MetaballEngine<SIZE> me;
    size_t m1 = me.add_metaball(glm::vec3(0.0f), tune_plane(0.f, 1.f, 0.f));
    size_t m2 = me.add_metaball(glm::vec3(1.f), tune_blob(0.6f, 0.6f, 3.f));
    size_t m3 = me.subtract_metaball(glm::vec3(1.f), tune_blob(0.6f, 0.6f, 3.f));
    me = me.refresh();

    Metaball& animated = me.get_metaball(m2);
    Metaball& anim2 = me.get_metaball(m3);

    MeshView mv = MeshView {&me.get_vertices(), &me.get_indices() };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mv.vertex_data->size() * sizeof(Vertex), mv.vertex_data->data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mv.indices->size() * sizeof(GLuint), mv.indices->data(), GL_STATIC_DRAW);

    Shader s = Shader::from_file(
        "./src/shaders/vertex/vertex_lighting.vert",
        "./src/shaders/fragment/vertex_lighting.frag"
    ).value();

    s.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
    });

    s.add_uniform("color", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(0.9f)[0]);
    });

    GLuint program = s.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");
    const GLint vnorm_location = glGetAttribLocation(program, "pNorm");

    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(vnorm_location);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.f),
        (float) SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.f
    );

    float lastFrame = 0.0f;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    const float FPS = 1.f / 30.f;
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(win, deltaTime);

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = (float)glfwGetTime();
        //glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view /** model*/;

        s.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });

        animated.position = 2.5f * glm::vec3(/*std::cos(currentFrame)*/ 0.f, 0.f, std::sin(currentFrame));
        anim2.position = -2.5f * glm::vec3(0.f, 0.f, std::sin(currentFrame));
        
        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        re_render_metaball_engine(me, mv, vbo, ebo);
        glDrawElements(GL_TRIANGLES, (GLsizei) mv.indices->size(), GL_UNSIGNED_INT, 0);
        
        glfwPollEvents();
        glfwSwapBuffers(win);
    }
    
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}

int help() {
    std::cout << "Please include a flag after the executable name. The options are:\n"
    << "\t1) -animation | -a : Play an animation of a paper folding/shaking that uses metaballs\n"
    << "\t2) -scenes | -s : View multiple rendered metaball scenes. Click left & right to change the scene. "
    << "Move the camera by dragging w/ the mouse left-click, and travel around the scene with WASD"
    << std::endl;
    return EXIT_SUCCESS;
}


#include <fieldrange.hpp>
#include <isosurface.hpp>
#include <intrange.hpp>
#include <metaball.hpp>
#include <metaball_traits.hpp>
#include <marcher.hpp>
#include <engine.hpp>

#include <memory>
#include <iostream>
#include <numeric>
#include <iterator>

template <typename T> using Unique = std::unique_ptr<T>;
template <typename T> using Ptr = T*;

struct InverseSquareBlob {
    glm::vec3 m_center = glm::vec3(0.0f);
    float m_scale = 1.0;

    InverseSquareBlob(const glm::vec3& center = glm::vec3(0.0), const float scale = 1.0f) 
        : m_center(center), m_scale(scale) {}

    float operator()(float x, float y, float z) const {
        return m_scale / (
            (float) std::pow(m_center.x - x, 2) + 
            (float) std::pow(m_center.y - y, 2) + 
            (float) std::pow(m_center.z - z, 2)
        );
    }

    BoundingBox get_bounding_box() const {
        const glm::vec3 sqrt_of_scale_vec(sqrtf(m_scale));
        return BoundingBox{ m_center + sqrt_of_scale_vec, m_center - sqrt_of_scale_vec };
    }
};

struct Gaussian {
    float variance = 1.0f;

    float operator()(float x, float y, float z) const {
        return expf(-(x*x + y*y + z*z) / (2*variance));
    }
};

float sum_floats(float x, float y, float z) {
    return x + y + z;
}

void print_bits(int bytes, int min_bits = 8, bool newline_terminate = false) {
    int initial = bytes;
    while (bytes != 0) {
        for (int i = 0; i < min_bits; i++) {
            std::cout << (bytes & 0x1);
            bytes = bytes >> 1;
        }
    }

    if (newline_terminate && initial != 0) {
        std::cout << std::endl;
    }
}

template <typename M>
glm::vec3 compute_gradient_main(const mbl::Metaball<M>& m, const glm::vec3& p, std::array<float, 6>& sns, const float eps = 1e-3f) {
    const glm::vec3 dx = glm::vec3(eps, 0, 0);
    const glm::vec3 dy = glm::vec3(0, eps, 0);
    const glm::vec3 dz = glm::vec3(0, 0, eps);

    const glm::vec3 pdx = p + dx;
    const glm::vec3 mdx = p - dx;
    const glm::vec3 pdy = p + dy;
    const glm::vec3 mdy = p - dy;
    const glm::vec3 pdz = p + dz;
    const glm::vec3 mdz = p - dx;

    sns[0] = m(pdx.x, pdx.y, pdx.z);
    sns[1] = m(mdx.x, mdx.y, mdx.z);
    sns[2] = m(pdy.x, pdy.y, pdy.z);
    sns[3] = m(mdy.x, mdy.y, mdy.z);
    sns[4] = m(pdz.x, pdz.y, pdz.z);
    sns[5] = m(mdz.x, mdz.y, mdz.z);

    return glm::vec3(
        sns[0] - sns[1],
        sns[2] - sns[3],
        sns[4] - sns[5]
    );
}

template <typename M>
glm::vec3 compute_normal_main(const mbl::Metaball<M>& m, const glm::vec3& p, std::array<float, 6>& sns, float eps = 1e-3f) {
    return -glm::normalize(compute_gradient_main(m, p, sns));
}

/** 
 *  What I learned from some of these benchmarks:
 *  - CRTP & Type Erasure Approaches seem to perform better than Type Erasure (Aggregate Metaball) and Vector of Functions as 'partitions' increases
 *  - As the 'length' increases, so too does the time each run takes (not sure why?)
 * 
 *  TODO:
 *  - Implement a 'joined' approach that merges the construct & density setting phases together
 *  - Export results to .csv and graph
 */
int test() {
    float length = 1;
    int32_t partitions = 5;
    float iso_value = 1.0f;

    mbl::Metaball<InverseSquareBlob> m1 = mbl::Metaball(InverseSquareBlob(glm::vec3(0.0f, 0.f, 0.f), 1.f));
    mbl::IsoSurface s = mbl::IsoSurface::construct(glm::vec3(0.f, 0.f, 0.f), (float) length, partitions);

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    std::cout << "Length = " << s.length() << std::endl;
    std::cout << "Origin = " << s.get_origin().x << ", " << s.get_origin().y << ", " << s.get_origin().z << std::endl;

    // Set density values for each IsoPoint
    for (mbl::IsoPoint& p : s.isopoints()) {
        glm::vec3& pos = p.position;
        std::cout << "[" << pos.x << "," << pos.y << "," << pos.z << "]" << std::endl;
        p.density = m1(p.position.x, p.position.y, p.position.z);
    }

    int empty_count = 0;
    int non_empty_count = 0;
    int total_cubes = 0;

    // Index offsets for index bit construction
    // maps my cube index ordering to that in the original
    // marching cubes paper (circling around the square instead of a Z-ordering)
    IndexDim offsets[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    // Take our points (re-ordered according to `offsets`) and store pointers to them in this buffer
    std::array<mbl::IsoPoint*,8> cube_isopoints = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    std::array<glm::vec3,12> edge_points = {};
    std::array<Vertex,12> cube_out_vertices = {}; // Vertices are passed here before finally being copied into the big array of Vertices
    std::array<int, 12> cube_out_indices = {};
    std::array<float, 6> sns = {};

    // Build the vertices for each cube and add to Vertex list
    for (mbl::CubeView cv : mbl::MarchingCubeRange(s)) {
        total_cubes += 1;

        uint8_t bits = 0;
        uint8_t mask = 1;

        // Compute the cube index for the cube
        uint8_t cube_isopoint_index = 0;
        for (IndexDim& offset : offsets) {
            mbl::IsoPoint& cube_point = cv.at(offset.x, offset.y, offset.z);
            bits = bits | (mask * (uint8_t) (cube_point.density >= iso_value));
            mask = mask << 1;

            cube_isopoints[cube_isopoint_index] = &cube_point;
            cube_isopoint_index += 1;
        }

        // for (mbl::IsoPoint& p : cv) {
        //     bits = bits | (mask * (uint8_t) (p.density >= 1));
        //     mask = mask << 1;
        // }

        print_bits(bits, 8, true);
        
        const bool outcome = (bits == 0 || bits == 0xFF);
        empty_count += (int) outcome;
        non_empty_count += (int) (!outcome);
        // std::cout << " Has Vertex = " << std::boolalpha << (!outcome) << std::endl;

        // If we have a non-trivial bit arrangement (bits != 0x0 || 0xFF) we can go on to get the vertices
        if (bits != 0x0 && bits != 0xFF) {

            // Get the interpolated edge midpoint positions
            int cube_edges = edge_table[bits];
            // std::cout << "\n===============\nEdge Bits = ";
            print_bits(cube_edges, 12, true);
            int cube_edge_index = 0;
            while (cube_edges != 0) {
                if ((0x1 & cube_edges) == 1) {
                    const int (&edge)[2] = edge_mappings[cube_edge_index];
                    // std::cout << "\nEdge #" << cube_edge_index << " = (" << edge[0] << ", "<< edge[1] << ")" << std::endl;

                    mbl::IsoPoint& I1 = *cube_isopoints[edge[0]];
                    mbl::IsoPoint& I2 = *cube_isopoints[edge[1]];

                    // std::cout << "I1 = [" << I1.position.x << ", " << I1.position.y << ", " << I1.position.z << "], Density = " << I1.density << std::endl;
                    // std::cout << "I2 = [" << I2.position.x << ", " << I2.position.y << ", " << I2.position.z << "], Density = " << I2.density << std::endl; 
    
                    float denominator = I2.density - I1.density;
                    edge_points[cube_edge_index] = I1.position + (iso_value - I1.density) * (I2.position - I1.position) / denominator;

                    //std::cout << "Edge #" << cube_edge_index << " Position = [" << edge_points[cube_edge_index].x 
                    //    << ", " << edge_points[cube_edge_index].y << ", " << edge_points[cube_edge_index].z << "]" << std::endl;
                }

                cube_edges = cube_edges >> 1;
                cube_edge_index += 1;
            }

            // Now construct the vertices and add to our Vertex buffer
            const int (&edge_ordering)[16] = triTable[bits];
            int edge_ordering_index = 0;
            while (edge_ordering[edge_ordering_index] != -1 && edge_ordering_index < 16) {
                const glm::vec3& p0 = edge_points[edge_ordering[edge_ordering_index]];
                const glm::vec3& p1 = edge_points[edge_ordering[edge_ordering_index+1]];
                const glm::vec3& p2 = edge_points[edge_ordering[edge_ordering_index+2]];

                cube_out_vertices[edge_ordering_index].position = edge_points[edge_ordering[edge_ordering_index]];
                cube_out_vertices[edge_ordering_index].normal = compute_normal_main(m1, cube_out_vertices[edge_ordering_index].position, sns); 

                cube_out_vertices[edge_ordering_index + 1].position = edge_points[edge_ordering[edge_ordering_index+1]];
                cube_out_vertices[edge_ordering_index + 1].normal = compute_normal_main(m1, cube_out_vertices[edge_ordering_index+1].position, sns); 

                cube_out_vertices[edge_ordering_index + 2].position = edge_points[edge_ordering[edge_ordering_index+2]];
                cube_out_vertices[edge_ordering_index + 2].normal = compute_normal_main(m1, cube_out_vertices[edge_ordering_index+2].position, sns); 

                int index_at = edge_ordering_index + (int) indices.size();
                cube_out_indices[edge_ordering_index] = index_at;
                cube_out_indices[edge_ordering_index + 1] = index_at + 1;
                cube_out_indices[edge_ordering_index + 2] = index_at + 2;

                edge_ordering_index += 3;
            }

            // copy from 'cube_out_vertices' to the full vertices buffer
            // std::cout << "\nCopying over " << edge_ordering_index << " vertices..." << std::endl;
            // for (int i = 0; i < edge_ordering_index; i++) {
            //    std::cout << "[" << cube_out_vertices[i].position.x << ", " << cube_out_vertices[i].position.y << ", " << cube_out_vertices[i].position.z << "]" << std::endl;
            // }

            std::copy(cube_out_vertices.begin(), cube_out_vertices.begin() + edge_ordering_index, std::back_inserter(vertices));
            std::copy(cube_out_indices.begin(), cube_out_indices.begin() + edge_ordering_index, std::back_inserter(indices));
        }
    }

    // std::cout << "Total Cubes = " << total_cubes << std::endl;
    // std::cout << "Empty Cubes (Exterior) = " << empty_count << std::endl;
    // std::cout << "Non-Empty Cubes = " << non_empty_count << std::endl;
    // std::cout << "vertice length = " << vertices.size() << ", indices length = " << indices.size() << std::endl;

    int k = 0;
    for (Vertex& v : vertices) {
        if (k > 0 && k % 3 == 0) {
            std::cout << " Indices = "; 
            for (int i = k - 3; i < k; i++) {
                std::cout << indices[i] << " ";
            }
            std::cout << "\n" << std::endl;
        }

        std::cout << "Vertex #" << k << "= Position: [" << v.position.x << ", " << v.position.y << ", " << v.position.z << "]"
                << " , Normal: [" << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << "]" << std::endl;
         k += 1;
    }

    // render loop

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes (Refactor) Test").open();

    // GLuint vao;
    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);

    // GLuint vbo;
    // glGenBuffers(1, &vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // GLuint ebo;
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    unsigned int VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object first
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Copy vertex data into VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Copy index data into EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    
    Shader shader = Shader::from_file(
        "./src/shaders/vertex/vertex_lighting.vert",
        "./src/shaders/fragment/vertex_lighting.frag"
    ).value();

    shader.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
    });

    shader.add_uniform("color", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(1.f, 0.f, 0.f)[0]);
    });

    GLuint program = shader.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");
    const GLint vnorm_location = glGetAttribLocation(program, "pNorm");

    // Position attribute
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(vpos_location);
    
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(vnorm_location);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.f),
        (float) SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.f
    );

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.0f);

    const float FPS = 1.f / 30.f;
    float lastFrame = 0.f;
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(win, deltaTime);

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view;

        shader.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });

        shader.ping_all_uniforms().use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);
        // glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);
        
        glfwPollEvents();
        glfwSwapBuffers(win);
    }
    
    glfwDestroyWindow(win);
    glfwTerminate();

    return EXIT_SUCCESS;
}

int test_2() {
    mbl::MetaballEngine<mbl::Metaball<InverseSquareBlob>> engine(glm::vec3(0.f), 2.f, 5, 1.f);
    engine.add_metaball(mbl::Metaball(InverseSquareBlob(glm::vec3(0.f), 1.f)));

    mbl::common::MeshData md = engine.construct_mesh();

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes (Refactor) Test").open();

    unsigned int VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object first
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Copy vertex data into VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(Vertex), md.vertices.data(), GL_STATIC_DRAW);

    // Copy index data into EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, md.indices.size() * sizeof(int), md.indices.data(), GL_STATIC_DRAW);

    
    Shader shader = Shader::from_file(
        "./src/shaders/vertex/vertex_lighting.vert",
        "./src/shaders/fragment/vertex_lighting.frag"
    ).value();

    shader.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
    });

    shader.add_uniform("color", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(1.f, 0.f, 0.f)[0]);
    });

    GLuint program = shader.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");
    const GLint vnorm_location = glGetAttribLocation(program, "pNorm");

    // Position attribute
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(vpos_location);
    
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(vnorm_location);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.f),
        (float) SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.f
    );

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.0f);

    const float FPS = 1.f / 30.f;
    float lastFrame = 0.f;
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(win, deltaTime);

        int width, height;
        glfwGetFramebufferSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view;

        shader.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });

        shader.ping_all_uniforms().use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei) md.indices.size(), GL_UNSIGNED_INT, 0);
        // glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);
        
        glfwPollEvents();
        glfwSwapBuffers(win);
    }
    
    glfwDestroyWindow(win);
    glfwTerminate();

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    int (*scenario)() = nullptr;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-animation" || arg == "-a") {
            scenario = animation_test;
        } else if (arg == "-scenes" || arg == "-s") {
            scenario = metaball_scenes;
        } else if (arg == "-help" || arg == "-h") {
            scenario = help;
        } else if (arg == "-test" || arg == "-t") {
            scenario = test_2;
        }
    }

    if (scenario != nullptr) {
        scenario();
    } else {
        std::cout << "Not a valid input." << std::endl;
        help();
    }
   
    return EXIT_SUCCESS;
}
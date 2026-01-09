
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

#include "dependencies/glm/gtc/random.hpp"

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

struct KineticBlob {
    glm::vec3 m_center = glm::vec3(0.f);
    glm::vec3 m_velocity = glm::vec3(0.f);
    float m_scale = 1.f;

    KineticBlob(const glm::vec3& center = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0), const float scale = 1.0f) 
        : m_center(center), m_velocity(velocity), m_scale(scale) {}

    float operator()(float x, float y, float z) const {
        return m_scale / (
            (float) std::pow(m_center.x - x, 2) + 
            (float) std::pow(m_center.y - y, 2) + 
            (float) std::pow(m_center.z - z, 2)
        );
    }

    glm::vec3& update(const float dt) {
        m_center = m_center + m_velocity * dt;
        return m_center;
    }
};

int test() {
    const glm::vec3 center = glm::vec3(0.f);
    const float side_length = 10.f;
    const int32_t resolution = 30;
    const float iso_value = 1.f;
    const int32_t num_metaballs = 10;

    mbl::MetaballEngine<mbl::Metaball<KineticBlob>> engine(center, side_length, resolution, iso_value);
    for (int i = 0; i < num_metaballs; i++) {
        glm::vec3 position = glm::linearRand(glm::vec3(-5.f), glm::vec3(5.f));
        glm::vec3 velocity = glm::sphericalRand(1.f);
        engine.add_metaball(mbl::Metaball(KineticBlob(position, velocity)));
    }

    mbl::common::graphics::MeshData md = engine.construct_mesh();

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
        "./src/shaders/fragment/vertex_lighting_rgb.frag"
    ).value();

    shader.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
    });

    shader.add_uniform("color", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(1.f, 0.f, 0.f)[0]);
    });

    glm::vec3 camera_pos = camera.position;
    shader.add_uniform("camera_pos", [&camera_pos](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &camera_pos[0]);
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

        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view;

        for (int i = 0; i < num_metaballs; i++) {
            KineticBlob& kb = engine.get_metaball((size_t) i).unwrap();
            glm::vec3& kb_pos = kb.update(deltaTime);

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
        
        engine.make_dirty();
        md = engine.construct_mesh();
        
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(Vertex), md.vertices.data(), GL_STATIC_DRAW);

        // Copy index data into EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, md.indices.size() * sizeof(int), md.indices.data(), GL_STATIC_DRAW);
 
        shader.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });

        glm::vec3 camera_pos = camera.position;
        shader.add_uniform("camera_pos", [&camera_pos](GLuint pgrm, GLint loc) {
            glUniform3fv(loc, 1, &camera_pos[0]);
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
            scenario = test;
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
#include <iostream>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <array>
#include <vector>

#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext/matrix_clip_space.hpp"
#include "dependencies/glm/ext/matrix_transform.hpp"
#include "dependencies/glm/gtc/type_ptr.hpp"

#include "dependencies/glfw-3.4/deps/glad/gl.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"

#include "utils/Chest.hpp"
#include "utils/NumericRange.hpp"

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

int main() {
    metaball_scenes();
    // animation_test();
    return EXIT_SUCCESS;
}


// #include <ratio>

// int ratio_test() {
//     std::ratio<10, 1> my_ratio = std::ratio<10, 1>();
//     constexpr int res = my_ratio.num();

//     return 0;
// }

// #include <iostream>
// #include <sstream>
// #include <string>

// // T needs to be copyable
// template <typename T>
// void print_num_bytes(T type) {
//     std::size_t bytes = sizeof(T);
//     std::cout << bytes << " bytes" << std::endl;
// }

// template <typename T>
// void print_bitstring(T type) {
//     const std::size_t bytes = sizeof(T);
//     const std::size_t total_bits = bytes * 8;
//     std::size_t bits_left = bytes * 8;
//     std::stringstream ss;
    
//     while (bits_left) {
//         int bit = (int) (type & 0x1);
//         type = type >> 1;
//         ss << bit;
//         bits_left -= 1;
//     }
    
//     std::cout << ss.str() << std::endl;
// }

// typedef void (*callback)(int bit_idx);

// // does nothing
// void V(int bit_idx) {}

// template <typename T>
// void callback_on_ones(T type, callback C) {
//     const size_t bytes = sizeof(T);
//     const size_t total_bits = bytes * 8;
//     size_t bits_left = total_bits;

//     std::array<callback, 2> callback_switch = {V, C};

//     while (bits_left) {
//         int bit = (int) (type & 0x1);
//         type = type >> 1;
//         callback_switch[bit](static_cast<int>(total_bits - bits_left));
//         bits_left -= 1;
//     }
// }

// template <typename T>
// void cback_on_ones(T type, callback C) {
//     size_t bit_index = 0;
//     std::array<callback, 2> cback_switch = {V, C};

//     while (type) {
//         int bit = (int) (type & 0x1);
//         type = type >> 1;
//         cback_switch[bit]((int) bit_index);
//         bit_index += 1;
//     }
// }

// void P(int bit_idx) {
//     std::cout << "bit at bit-index: " << bit_idx << std::endl;
// }

// int main() {
//     int n1 = 10;
//     char c = 'a';
//     callback_on_ones(n1, P);
//     cback_on_ones(n1, P);
//     return 0;
// }
 
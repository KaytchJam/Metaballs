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

void re_render_metaball_engine(MetaballEngine& me, GLuint& vbo, GLuint& ebo) {
    me.refresh();
    const std::vector<Vertex>& vertex_data = me.get_vertices();
    const std::vector<GLuint>& indices = me.get_indices();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(Vertex), vertex_data.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
}

template<size_t SCENES>
struct MetaballSceneViewer {
    size_t scene_at = 0;
    std::array<MetaballEngine, SCENES> scenes;
    std::array<glm::vec3, SCENES> mball_color;

    MetaballEngine& get_current_scene() {
        return this->scenes[this->scene_at];
    }

    const glm::vec3& get_current_color() {
        return this->mball_color[this->scene_at];
    }

    MetaballSceneViewer& shift_left() {
        if (this->scene_at == 0) {
            this->scene_at = SCENES - 1;
        } else {
            this->scene_at -= 1;
        }

        return *this;
    }

    MetaballSceneViewer& shift_right() {
        if (this->scene_at == SCENES - 1) {
            this->scene_at = 0;
        } else {
            this->scene_at += 1;
        }

        return *this;
    }
};

template<size_t SCENES>
using MSV = MetaballSceneViewer<SCENES>;

MSV<6> setup_scenes() {
    MSV<6> mball_scenes;

    mball_scenes.mball_color[0] = glm::vec3(0.f, 0.f, 1.f);
    MetaballEngine* m = &mball_scenes.scenes[0];
    m->add_metaball(glm::vec3(0.f)); // THE SIMPLE SCENE

    mball_scenes.mball_color[1] = glm::vec3(0.f, 1.f, 0.f);
    m = &mball_scenes.scenes[1];
    m->add_metaball(glm::vec3(0.f), tune_blob(2.f, 5.f, 1.f));
    m->add_metaball(glm::vec3(1.7f), tune_blob(8.f, 2.f, 2.f));
    m->add_metaball(glm::vec3(0.0, 2.0, 0.9f), tune_blob(10.f, 1.5f, 2.f));

    mball_scenes.mball_color[2] = glm::vec3(1.f, 0.f, 0.f);
    m = &mball_scenes.scenes[2];
    m->add_metaball(glm::vec3(0.f), tune_cube(1.f, 1.f, 1.f, 0.f));

    mball_scenes.mball_color[3] = glm::vec3(1.f, 0.f, 1.f);
    m = &mball_scenes.scenes[3];
    m->add_metaball(glm::vec3(1.2f), tune_cube(1.f, 1.f, 1.f, 0.f));
    m->add_metaball(glm::vec3(1.9f), tune_blob(1.f, 2.f, 3.f));

    mball_scenes.mball_color[4] = glm::vec3(0.8f, 0.8f, 0.8f);
    m = &mball_scenes.scenes[4];
    m->add_metaball(glm::vec3(0.0f), tune_wave());

    mball_scenes.mball_color[5] = glm::vec3(0.8f, 0.1f, 0.6f);
    m = &mball_scenes.scenes[5];
    m->add_metaball(glm::vec3(0.0f), tune_blob(1.f, 1.f, 1.f));
    m->add_metaball(glm::vec3(0.4f, 0.5f, 0.1f), tune_cube(1.f, 1.f, 1.f, 0.f));
    m->add_metaball(glm::vec3(-0.3f, 2.1f, 1.4f), tune_cube(1.f, 1.f, 1.f, 0.f));

    return mball_scenes;
}

template <size_t S>
void check_keypress(GLFWwindow* w, MSV<S>& scenes) {
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

    MSV<6> scenes = setup_scenes();
    MetaballEngine* me = &scenes.get_current_scene();
    size_t prev_scene = scenes.scene_at;

    const std::vector<Vertex>* vertex_data = &me->get_vertices();
    const std::vector<GLuint>* indices = &me->get_indices();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data->size() * sizeof(Vertex), vertex_data->data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), indices->data(), GL_STATIC_DRAW);

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

    // std::cout << "render loop" << std::endl;

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float) glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(win, deltaTime);
        check_keypress(win, scenes);

        if (scenes.scene_at != prev_scene) {
            prev_scene = scenes.scene_at;
            re_render_metaball_engine(scenes.get_current_scene(), vbo, ebo);
            indices = &scenes.get_current_scene().get_indices();
            vertex_data = &scenes.get_current_scene().get_vertices();

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
        glDrawElements(GL_TRIANGLES, (GLsizei) indices->size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
 
int main() {
    metaball_scenes();
    return EXIT_SUCCESS;
}
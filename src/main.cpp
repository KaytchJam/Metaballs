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
#include "dependencies/glfw-3.4/deps/linmath.h"

#include "utils/Chest.hpp"
#include "utils/NumericRange.hpp"

#include "Shader.hpp"
#include "Metaball.hpp"

template <int v_size, typename T>
void print_vec(const glm::vec<v_size, T, glm::packed_highp>& v) {
    std::cout << "[";
    if (v_size > 0) std::cout << v[0];
    for (int i = 1; i < v_size; i++) {
        std::cout << ", " << v[i];
    }
    std::cout << "]";
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct Camera {
    glm::vec3 position = { 2.5f, 2.0f, 3.0f };
    glm::vec3 front = { 0.0f, 0.0f, -1.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };

    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 2.5f;
    float sensitivity = 0.1f;

    glm::mat4 get_view() const {
        return glm::lookAt(position, position + front, up);
    }
};

Camera camera;
float lastX = 320, lastY = 240;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos_d, double ypos_d) {
    if (glfwGetWindowAttrib(window, GLFW_HOVERED) == GLFW_FALSE)
        return;

    const float xpos = (float) xpos_d;
    const float ypos = (float) ypos_d;

    if (firstMouse) {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= camera.sensitivity;
    yoffset *= camera.sensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    direction.y = sin(glm::radians(camera.pitch));
    direction.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    camera.front = glm::normalize(direction);
}

void process_input(GLFWwindow* window, float deltaTime) {
    float velocity = camera.speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.position += velocity * camera.front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.position -= velocity * camera.front;

    glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.position += right * velocity;
}

// Modify setup to set cursor callback
Chest<GLFWwindow*> setup(int length, int width, const char* name) {
    Chest<GLFWwindow*> winchest = Chest<GLFWwindow*>::sign("");

    glfwSetErrorCallback(error_callback);
    if (!ErrorInt(glfwInit(), GLFW_FALSE)) {
        winchest.sign_append_newline("[1] glfwInit() call failed.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(length, width, name, nullptr, nullptr);
    if (win == nullptr) {
        glfwTerminate();
        winchest.sign_append_newline("[2] glfwCreateWindow call failed.");
    } else {
        glfwSetKeyCallback(win, key_callback);
        glfwMakeContextCurrent(win);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);
        glfwSetCursorPosCallback(win, mouse_callback);
        glfwSetErrorCallback(error_callback);
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        winchest = Chest<GLFWwindow*>::stuff(win);
    }

    return winchest;
}

auto tune_blob(float c1, float c2, float c3) {
    return [c1, c2, c3](const glm::vec3& center, const glm::vec3& pt) {
        return 1.f / (
        c1 * std::powf(center.x - pt.x, 2) + 
        c2 * std::powf(center.y - pt.y, 2) + 
        c3 * std::powf(center.z - pt.z, 2));
    };
}

void re_render_metaball_engine(MetaballEngine& me, GLuint& vbo, GLuint& ebo) {
    me.refresh();
    const std::vector<Vertex>& vertex_data = me.get_vertices();
    const std::vector<GLuint>& indices = me.get_indices();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(Vertex), vertex_data.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
}

int t6() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes Example").open();

    MetaballEngine me;
    size_t m1 = me.add_metaball(glm::vec3(0.0f));
    size_t m2 = me.add_metaball(glm::vec3(1.f), tune_blob(2.f, 2.f, 10.f));
    me = me.refresh();

    Metaball& animated = me.get_metaball(m2);
    // translation here

    const std::vector<Vertex>& vertex_data = me.get_vertices();
    const std::vector<GLuint>& indices = me.get_indices();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(Vertex), vertex_data.data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    Shader s = Shader::from_file(
        "./src/shaders/vertex/vertex_lighting.vert",
        "./src/shaders/fragment/vertex_lighting.frag"
    ).value();

    s.add_uniform("lightPos", [](GLuint pgrm, GLint loc) {
        glUniform3fv(loc, 1, &glm::vec3(10.f, 10.f, 10.f)[0]);
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

    std::cout << "render loop" << std::endl;

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
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        glm::mat4 view = camera.get_view();
        glm::mat4 mvp = proj * view * model;

        s.add_uniform("MVP", [mvp](GLuint prog, GLint loc) { 
            glUniformMatrix4fv(loc, 1, false, glm::value_ptr(mvp)); 
        });

        animated.position = glm::vec3(std::cos(currentFrame), 0.f, std::sin(currentFrame)) * deltaTime;
        
        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        re_render_metaball_engine(me, vbo, ebo);
        glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
 
int main() {
    t6();
    // std::cout << "\nExiting." << std::endl;
    return EXIT_SUCCESS;
}
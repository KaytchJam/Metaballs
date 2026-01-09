#pragma once

#include "dependencies/glm/ext/matrix_transform.hpp"

#include "dependencies/glfw-3.4/deps/glad/gl.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"

#include "utils/Chest.hpp"
#include "Shader.hpp"

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
    glm::vec3 position = { 0.0, 0.0, -5 };
    glm::vec3 front = { 0.0f, 0.0f, 1.0f };
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
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        firstMouse = true;
    }

    if (glfwGetWindowAttrib(window, GLFW_HOVERED) == GLFW_FALSE)
        return;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
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
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        winchest = Chest<GLFWwindow*>::stuff(win);
    }

    return winchest;
}

auto tune_blob(float c1 = 1.f, float c2 = 1.f, float c3 = 1.f) {
    return [c1, c2, c3](const glm::vec3& center, const glm::vec3& pt) -> float {
        return 1.f / (
        c1 * (float) std::pow(center.x - pt.x, 2) + 
        c2 * (float) std::pow(center.y - pt.y, 2) + 
        c3 * (float) std::pow(center.z - pt.z, 2));
    };
}

auto tune_cube(float c1 = 1.f, float c2 = 1.f, float c3 = 1.f, float eps = 0.f) {
    return [c1, c2, c3, eps](const glm::vec3& center, const glm::vec3& pt) -> float {
        return 1.f / (
            c1 * (float) std::pow(center.x - pt.x, 4) + 
            c2 * (float) std::pow(center.y - pt.y, 4) + 
            c3 * (float) std::pow(center.z - pt.z, 4) +
            eps
        );
    };
}

auto tune_gyroid(float a1 = 1.f, float a2 = 1.f, float a3 = 1.f) {
    using namespace std;
    return [a1, a2, a3](const glm::vec3& center, const glm::vec3& pt) -> float {
        return a1 * sin(pt.x)*cos(pt.y) + a2 * sin(pt.y)*cos(pt.z) + a3 * sin(pt.z)*cos(pt.x);
    };
}

auto tune_cross(float c1 = 1.f, float c2 = 1.f, float c3 = 1.f) {
    return [c1, c2, c3](const glm::vec3& center, const glm::vec3& pt) -> float {
        return c1/ ((float) std::pow(center.x - pt.x, 2)) + c2/ ((float) std::pow(center.y - pt.y, 2)) + c3/ ((float) std::pow(center.z - pt.z, 2));
    };
}

auto tune_plane(float c1 = 1.f, float c2 = 1.f, float c3 = 1.f, float offset = 1.f) {
    return [c1, c2, c3, offset](const glm::vec3& center, const glm::vec3& pt) -> float {
        return c1 * (center.x - pt.x) + c2 * (center.y - pt.y) + c3 * (center.z - pt.z) + offset;
    };
}

auto tune_star(float scale = 1.f) {
    using namespace std;
    return [scale](const glm::vec3& center, const glm::vec3& pt) -> float {
        return scale / abs((center.x - pt.x) * (center.y - pt.y));
    };
}

auto tune_paraboliod(float a1 = 1.f, float a2 = 1.f, float a3 = 1.f, float thickness = 1.f) {
    using namespace std;
    return [a1, a2, a3, thickness](const glm::vec3& center, const glm::vec3& pt) -> float {
        return (float) abs( thickness / (pow((center.x - pt.x) * a1, 2) + pow((center.z - pt.z) * a2, 2) + a3 * (center.y - pt.y)));
    };
}
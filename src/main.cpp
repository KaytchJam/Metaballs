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
#include "MarchingCubes.hpp"

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

int t2() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple Cube").open();

    // first 3 = position, second 3 = color
    float vertices[] = {
        1.f, 1.f, 1.f, 1.0f, 0.0f, 0.0f, //0
        -1.f, 1.f, 1.f, 0.0f, 1.0f, 0.0f, //1
        -1.f, 1.f, -1.f, 0.0f, 0.0f, 1.0f, //2
        1.f, 1.f, -1.f, 1.0f, 1.0f, 1.0f, //3
        1.f, -1.f, 1.f, 1.0f, 1.0f, 0.0f, //4
        -1.f, -1.f, 1.f, 1.0f, 1.0f, 1.0f, //5
        -1.f, -1.f, -1.f, 0.0f, 1.0f, 1.0f, //6
        1.f, -1.f, -1.f, 1.0f, 0.0f, 1.0f //7
    };

    GLuint indices[] = {
        0, 1, 3, //top 1
        3, 1, 2, //top 2
        2, 6, 7, //front 1
        7, 3, 2, //front 2
        7, 6, 5, //bottom 1
        5, 4, 7, //bottom 2
        5, 1, 4, //back 1
        4, 1, 0, //back 2
        4, 3, 7, //right 1
        3, 4, 0, //right 2
        5, 6, 2, //left 1
        5, 1, 2  //left 2
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    Shader s = Shader::from_file(
        "./src/shaders/vertex/cube.vert",
        "./src/shaders/fragment/cube.frag"
    ).value();

    GLuint program = s.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "VertexPosition");
    const GLint vcol_location = glGetAttribLocation(program, "VertexColor");
    // const GLint vpos_location = 0;
    // const GLint vcol_location = 1;

    std::cout << "VertexPosition location: " << vpos_location << std::endl;
    std::cout << "VertexColor location: " << vcol_location << std::endl;

    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(vcol_location);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.f),
        (float) SCREEN_WIDTH / SCREEN_HEIGHT,
        0.1f,
        100.f
    );

    float lastFrame = 0.0f;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

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

        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}

template <typename T, size_t X, size_t Y, size_t Z>
using Array3D = std::array<std::array<std::array<T,X>, Y>, Z>;

template <typename T, size_t S>
using ArrayCube = Array3D<T, S, S, S>;

struct control_point_t {
    glm::vec3 pos;
    float density;
    bool active;
};

// template <size_t CPC, size_t CPR, size_t CPL, typename ImplicitFunction>
// Array3D<control_point_t, CPC + 1, CPR + 1, CPL + 1> get_control_points(
//     const glm::vec3& CENTER,
//     const float CUBE_SIZE, 
//     const float ISOVALUE, 
//     ImplicitFunction F
// ) {
//     constexpr size_t LAYERS = CPL + 1;
//     constexpr size_t ROWS   = CPR + 1;
//     constexpr size_t COLS   = CPC + 1;

//     const glm::vec3 top_left = CENTER - glm::vec3(COLS, -1 * ((float)ROWS), LAYERS) * CUBE_SIZE / 2.f;

//     // std::cout << "top left is: ";
//     // print_vec(top_left);


//     // OBSERVATION: we're only concerned with two "layers" at a time
//     // assumption: the cube construct is centered around certain x & y  coords
//     Array3D<control_point_t, COLS, ROWS, LAYERS> cube_vertices;

//     glm::vec3 cube_tl = top_left;
//     for (size_t layer = 0; layer < LAYERS; layer++) {
//         cube_tl.y = top_left.y;
//         for (size_t row = 0; row < ROWS; row++) {
//             cube_tl.x = top_left.x;
//             for (size_t col = 0; col < COLS; col++) {

//                 const glm::vec3 v_pos = top_left + cube_tl;
//                 const float density = F(v_pos);
//                 cube_vertices[layer][row][col] = control_point_t {
//                     cube_tl,
//                     density,
//                     density >= ISOVALUE
//                 };

//                 cube_tl.x += CUBE_SIZE;
//             }
//             cube_tl.y -= CUBE_SIZE;
//         }
//         cube_tl.z += CUBE_SIZE;
//     }

//     return cube_vertices;
// }

template <typename ImplicitFunction>
glm::vec3 gradient_at(const glm::vec3& p, ImplicitFunction f, float eps = 1e-3f) {
    return glm::normalize(glm::vec3(
        f(p + glm::vec3(eps, 0, 0)) - f(p - glm::vec3(eps, 0, 0)),
        f(p + glm::vec3(0, eps, 0)) - f(p - glm::vec3(0, eps, 0)),
        f(p + glm::vec3(0, 0, eps)) - f(p - glm::vec3(0, 0, eps))
    ));
}

template <size_t CPC, size_t CPR, size_t CPL, typename ImplicitFunction>
Array3D<control_point_t, CPC + 1, CPR + 1, CPL + 1> get_control_points(
    const glm::vec3& CENTER,
    const float CUBE_SIZE, 
    const float ISOVALUE, 
    ImplicitFunction F
) {
    constexpr size_t LAYERS = CPL + 1;
    constexpr size_t ROWS   = CPR + 1;
    constexpr size_t COLS   = CPC + 1;
    const glm::vec3 grid_min = CENTER - glm::vec3(CPC, CPR, CPL) * CUBE_SIZE * 0.5f;

    Array3D<control_point_t, COLS, ROWS, LAYERS> cube_vertices;

    for (size_t z = 0; z < LAYERS; ++z) {
        for (size_t y = 0; y < ROWS; ++y) {
            for (size_t x = 0; x < COLS; ++x) {
                glm::vec3 pos = grid_min + glm::vec3(x, y, z) * CUBE_SIZE;
                float density = F(pos);
                cube_vertices[z][y][x] = control_point_t{
                    pos,
                    density,
                    density >= ISOVALUE
                };
            }
        }
    }

    return cube_vertices;
}

template <size_t COLS, size_t ROWS, size_t LAYERS>
uint8_t compute_cube_index(const Array3D<control_point_t, COLS, ROWS, LAYERS>& ctrl_verts, const glm::ivec3& at) {
    glm::ivec3 offsets[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    uint8_t index = 0;
    uint8_t iteration = 0;
    for (const glm::ivec3& offset : offsets) {
        const glm::ivec3 vertex_index = at + offset;
        const control_point_t& cp = ctrl_verts[vertex_index.z][vertex_index.y][vertex_index.x];

        if (cp.active) index |= (1 << iteration);  // i = 0 to 7
        iteration++;
    }

    return index;
}

template <size_t COLS, size_t ROWS, size_t LAYERS>
glm::vec3 average_cube(const Array3D<control_point_t, COLS, ROWS, LAYERS>& ctrl_verts, const glm::ivec3& at) {
    glm::ivec3 offsets[8] = {
        {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}
    };

    glm::vec3 center = glm::vec3(0.0f);
    for (const glm::ivec3& offset : offsets) {
        const glm::ivec3 vertex_index = at + offset;
        const control_point_t& cp = ctrl_verts[vertex_index.z][vertex_index.y][vertex_index.x];
        center += cp.pos;
    }

    return center / 8.0f;
}

// template <size_t COLS, size_t ROWS, size_t LAYERS, typename F, typename T>
// Array3D<MapResult<F,T>, COLS - 1, ROWS - 1, LAYERS - 1> map_cubes(const Array3D<T, COLS, ROWS, LAYERS>& a, F f) {

// }

// template <size_t COLS, size_t ROWS, size_t LAYERS, typename F>
// Array3D<MapResult<F,control_point_t>, COLS - 1, ROWS - 1, LAYERS - 1> get_cube_bits(const Array3D<control_point_t, COLS, ROWS, LAYERS>& ctrl_verts) {
//     Array3D<uint8_t, COLS - 1, ROWS - 1, LAYERS - 1> cube_bits;
//     for (size_t layer = 0; layer < LAYERS - 1; layer++) {
//         for (size_t row = 0; row < ROWS - 1; row++) {
//             for (size_t col = 0; col < COLS - 1; col++) {
//                 cube_bits[layer][row][col] = compute_cube_index(ctrl_verts, glm::ivec3(col, row, layer));
//             }
//         }
//     }

//     return cube_bits;
// }

template <size_t COLS, size_t ROWS, size_t LAYERS>
Array3D<uint8_t, COLS - 1, ROWS - 1, LAYERS - 1> get_cube_bits(const Array3D<control_point_t, COLS, ROWS, LAYERS>& ctrl_verts) {
    Array3D<uint8_t, COLS - 1, ROWS - 1, LAYERS - 1> cube_bits;
    for (size_t layer = 0; layer < LAYERS - 1; layer++) {
        for (size_t row = 0; row < ROWS - 1; row++) {
            for (size_t col = 0; col < COLS - 1; col++) {
                cube_bits[layer][row][col] = compute_cube_index(ctrl_verts, glm::ivec3(col, row, layer));
            }
        }
    }

    return cube_bits;
}

template <size_t COLS, size_t ROWS, size_t LAYERS>
Array3D<glm::vec3, COLS - 1, ROWS - 1, LAYERS - 1> average_cube_pts(const Array3D<control_point_t, COLS, ROWS, LAYERS>& ctrl_verts) {
    Array3D<glm::vec3, COLS - 1, ROWS - 1, LAYERS - 1> cube_midpoints;
    for (size_t layer = 0; layer < LAYERS - 1; layer++) {
        for (size_t row = 0; row < ROWS - 1; row++) {
            for (size_t col = 0; col < COLS - 1; col++) {
                cube_midpoints[layer][row][col] = average_cube(ctrl_verts, glm::ivec3(col, row, layer));
            }
        }
    }

    return cube_midpoints;
}

int t3() {
    constexpr size_t CPR = 10;
    constexpr size_t CPC = 10;
    constexpr size_t CPL = 10;

    glm::vec3 center = glm::vec3(0.f);
    auto metaball_func = [center](const glm::vec3 pt) {
        return 1.f / (std::powf(center.x - pt.x, 2) + std::powf(center.y - pt.y, 2) + std::powf(center.z - pt.z, 2));
    };

    const float ISOVALUE = 1.0f;
    const float CUBE_SIZE = 1.0f / 5.0f;

    Array3D<control_point_t, CPC + 1, CPR + 1, CPL + 1> c = get_control_points<CPC,CPR,CPL>(
        center,
        CUBE_SIZE,
        ISOVALUE,
        metaball_func
    );
    
    Array3D<uint8_t, CPC, CPR, CPL> tri_index = get_cube_bits(c);
    std::cout << "Check 1. Size = " << (CPC * CPR * CPL) << std::endl;
    for (size_t k = 0; k < CPL; k++) {
        for (size_t j = 0; j < CPR; j++) {
            for (size_t i = 0; i < CPC; i++) {
                std::cout << (int) (tri_index[k][j][i]) << " ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

std::pair<Array3D<uint8_t, 10, 10, 10>, Array3D<glm::vec3, 10,10,10>> get_metaball_pts() {
    constexpr size_t CPR = 10;
    constexpr size_t CPC = 10;
    constexpr size_t CPL = 10;

    glm::vec3 center = glm::vec3(0.f);

    auto metaball_func = [center](const glm::vec3 pt) {
        return 1.f / (std::powf(center.x - pt.x, 2) + std::powf(center.y - pt.y, 2) + std::powf(center.z - pt.z, 2));
    };

    const float ISOVALUE = 1.0f;
    const float CUBE_SIZE = 1.0f / 5.0f;

    Array3D<control_point_t, CPC + 1, CPR + 1, CPL + 1> c = get_control_points<CPC,CPR,CPL>(
        center,
        CUBE_SIZE,
        ISOVALUE,
        metaball_func
    );
    
    Array3D<uint8_t, CPC, CPR, CPL> tri_index = get_cube_bits(c);
    Array3D<glm::vec3, CPC, CPR, CPL> cube_midpoints = average_cube_pts(c);
    return std::pair(tri_index, cube_midpoints);
}

Array3D<glm::vec4, 10, 10, 10> join_isosurface_arrays(
    const Array3D<uint8_t, 10, 10, 10>& densities, 
    const Array3D<glm::vec3, 10, 10, 10>& vertices
) {
    Array3D<glm::vec4, 10, 10, 10> combined;

    for (size_t k = 0; k < 10; k++) {
        for (size_t j = 0; j < 10; j++) {
            for (size_t i = 0; i < 10; i++) {
                const glm::vec3& p = vertices[k][j][i];
                combined[k][j][i] = glm::vec4(p.x, p.y, p.z, (float) densities[k][j][i] / 255.f);
            }
        }
    }

    return combined;
}

ArrayCube<control_point_t, 11> get_metaball_control_points(const float isovalue) {
    constexpr size_t CPR = 10;
    constexpr size_t CPC = 10;
    constexpr size_t CPL = 10;

    glm::vec3 center = glm::vec3(0.f);

    auto metaball_func = [center](const glm::vec3 pt) {
        return 1.f / (std::powf(center.x - pt.x, 2) + std::powf(center.y - pt.y, 2) + std::powf(center.z - pt.z, 2));
    };

    const float CUBE_SIZE = 1.0f / 5.0f;

    Array3D<control_point_t, CPC + 1, CPR + 1, CPL + 1> c = get_control_points<CPC,CPR,CPL>(
        center,
        CUBE_SIZE,
        isovalue,
        metaball_func
    );

    std::cout << "top left: ";
    print_vec(c[0][0][0].pos);
    std::cout << "\nbottom right: ";
    print_vec(c[10][10][10].pos);
    std::cout << std::endl;
    
    return c;
}

template <size_t CUBE_SIZE>
std::array<control_point_t, 8> get_cube_cpts(const ArrayCube<control_point_t, CUBE_SIZE>& C, size_t i, size_t j, size_t k) {
    return std::array<control_point_t, 8> {
        C[k][j][i],
        C[k][j][i+1],
        C[k][j+1][i+1],
        C[k][j+1][i],
        C[k+1][j][i],
        C[k+1][j][i+1],
        C[k+1][j+1][i+1],
        C[k+1][j+1][i],
    };
}

std::array<glm::vec3, 12> calc_edge_lerps(
    const std::array<control_point_t,8>& cube_cpts, 
    const float isovalue,
    uint8_t edge_mask
) {
    std::array<glm::vec3, 12> lerp_set = {};
    int bit_index = 0;

    while (edge_mask != 0 && bit_index < 12) {
        //uint8_t bit = (edge_mask & 0x1);
    
        int e1 = edge_mappings[bit_index][0];
        int e2 = edge_mappings[bit_index][1];

        const glm::vec3& P1 = cube_cpts[e1].pos;
        const float V1 = cube_cpts[e1].density;

        const glm::vec3& P2 = cube_cpts[e2].pos;
        const float V2 = cube_cpts[e2].density;

        float denom = V2 - V1;
        if (std::abs(denom) < 1e-6f) denom = 1e-6f;
        lerp_set[bit_index] = P1 + (isovalue - V1) * (P2 - P1) / denom;

        //edge_mask = edge_mask >> 1;
        bit_index += 1;
    }

    return lerp_set;
}

struct MetaballBufferData {
    std::vector<glm::vec3> vertex_buffer;
    std::vector<glm::vec3> normal_buffer;
    std::vector<uint32_t> index_buffer;
};

MetaballBufferData construct_metaball_mesh() {
    const float ISOVALUE = 2.0f;
    std::vector<glm::vec3> vertex_buffer;
    std::vector<uint32_t> index_buffer;
    std::vector<glm::vec3> normal_buffer;

    glm::vec3 center = glm::vec3(0.f);
    auto metaball_func = [center](const glm::vec3 pt) {
        return 1.f / (std::powf(center.x - pt.x, 2) + std::powf(center.y - pt.y, 2) + std::powf(center.z - pt.z, 2));
    };

    ArrayCube<control_point_t, 11> cpts = get_metaball_control_points(ISOVALUE);

    for (size_t k = 0; k < 10; k++) {
        for (size_t j = 0; j < 10; j++) {
            for (size_t i = 0; i < 10; i++) {
                const uint8_t cube_index = compute_cube_index(cpts, glm::ivec3(i, j, k));
                const std::array<control_point_t, 8> cube_cpts = get_cube_cpts(cpts, i, j, k);
                const int cube_edge_mask = edge_masks[cube_index];
                const std::array<glm::vec3, 12> lerp_points = calc_edge_lerps(cube_cpts, ISOVALUE, cube_edge_mask);

                std::cout << "Cube #" << (int) cube_index << " produces " 
                    << (triTable[cube_index][0] == -1 ? 0 : 1) << " triangle set(s)\n";

                int edge_index = 0;
                while (triTable[cube_index][edge_index] != -1 && edge_index + 2 < 16) {

                    const glm::vec3& p0 = lerp_points[triTable[cube_index][edge_index]];
                    const glm::vec3& p1 = lerp_points[triTable[cube_index][edge_index+1]];
                    const glm::vec3& p2 = lerp_points[triTable[cube_index][edge_index+2]];

                    // update vertices
                    const uint32_t base_index = (uint32_t) vertex_buffer.size();
                    vertex_buffer.push_back(p0);
                    vertex_buffer.push_back(p1);
                    vertex_buffer.push_back(p2);

                    // update normals
                    normal_buffer.push_back(-gradient_at(p0, metaball_func));
                    normal_buffer.push_back(-gradient_at(p1, metaball_func));
                    normal_buffer.push_back(-gradient_at(p2, metaball_func));

                    // update indices
                    index_buffer.push_back(base_index);
                    index_buffer.push_back(base_index + 1);
                    index_buffer.push_back(base_index + 2);

                    edge_index += 3;
                }

                if (edge_index == 0) {
                    std::cout << "no triangle sets were appended" << std::endl;
                }
            }
        }
    }

    return MetaballBufferData { std::move(vertex_buffer), std::move(normal_buffer), std::move(index_buffer) };
}

int render_field() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const float ISOVALUE = 1.5f;

    ArrayCube<control_point_t, 11> cpts = get_metaball_control_points(ISOVALUE);

    ArrayCube<uint8_t, 10> tri_index = get_cube_bits(cpts);
    ArrayCube<glm::vec3, 10> cube_midpoints = average_cube_pts(cpts);
    ArrayCube<glm::vec4, 10> points = join_isosurface_arrays(tri_index, cube_midpoints);

    std::vector<glm::vec4> vertices;
    for (size_t i = 0; i < 10; i++) {
        for (size_t j = 0; j < 10; j++) {
            for (size_t k = 0; k < 10; k++) {
                vertices.push_back(points[i][j][k]);
                print_vec(points[i][j][k]);
                std::cout << std::endl;
            } 
        }
    }

    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple Cube").open();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_STATIC_DRAW);

    // GLuint ebo;
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    std::cout << "loading shaders" << std::endl;

    Shader s = Shader::from_file(
        "./src/shaders/vertex/point_cloud.vert",
        "./src/shaders/fragment/point_cloud.frag"
    ).value();

    std::cout << "setting attribs" << std::endl;

    GLuint program = s.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");

    glVertexAttribPointer(vpos_location, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glEnableVertexAttribArray(vpos_location);

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

        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, vertices.size());

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}

int t4() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes Example").open();
    
    MetaballBufferData mbd = construct_metaball_mesh();
    const std::vector<glm::vec3>& vertices = mbd.vertex_buffer;
    const std::vector<uint32_t>& indices = mbd.index_buffer;

    int count = 0;
    for (const glm::vec3& v : vertices) {
        if (count % 3 == 0) {
            std::cout << std::endl;
        }
        print_vec(v);
        count += 1;;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    Shader s = Shader::from_file(
        "./src/shaders/vertex/vertex_simple.vert",
        "./src/shaders/fragment/vertex_simple.frag"
    ).value();

    GLuint program = s.get_program_id();
    const GLint vpos_location = glGetAttribLocation(program, "pPos");

    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(vpos_location);

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

        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}

int t5() {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    GLFWwindow* win = setup(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Cubes Example").open();
    
    MetaballBufferData mbd = construct_metaball_mesh();
    const std::vector<glm::vec3>& vertices = mbd.vertex_buffer;
    const std::vector<uint32_t>& indices = mbd.index_buffer;
    const std::vector<glm::vec3>& normals = mbd.normal_buffer;

    std::vector<float> vertex_data;
    for (size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3& p = vertices[i];
        const glm::vec3& n = normals[i];
        vertex_data.insert(vertex_data.end(), { p.x, p.y, p.z, n.x, n.y, n.z });
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

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
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
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

        s.ping_all_uniforms().use();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
 
int main() {
    t5();
    // std::cout << "\nExiting." << std::endl;
    return EXIT_SUCCESS;
}
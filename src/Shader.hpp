#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>

#include "dependencies/glm/glm.hpp"
#include "dependencies/glfw-3.4/deps/glad/gl.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"

struct ErrorInt {
    int value = 0;
    int sentinel = 0;

    ErrorInt() : value(0), sentinel(0) {}
    ErrorInt(int sentinel) : value(0), sentinel(sentinel) {}
    ErrorInt(int sentinel, int result) : value(result), sentinel(sentinel) {}

    bool valid() const {
        return this->value != this->sentinel;
    }

    explicit operator bool() const {
        return this->valid();
    }

    int* v_ptr() {
        return &this->value;
    }
};

struct Uniform {
    GLint location;
    std::function<void(GLuint, GLint)> func;
};

// Shader abstraction to make using shaders ezpz
class Shader {
private:
    typedef std::unordered_map<std::string, Uniform> uniform_map;

    GLuint program_id = 0;
    uniform_map umap;

    // constants
    static constexpr int SHADER_COMPILE_FAIL = 0;
    static constexpr int PROGRAM_LINK_FAIL = 0;

    Shader() {}

    static ErrorInt create_shader(
        GLuint& shader_id, 
        const std::string& shader_src,
        const GLint SHADER_TYPE
    ) {
        shader_id = glCreateShader(SHADER_TYPE);
        const char* shader_src_ptr = shader_src.c_str();
        glShaderSource(shader_id, 1, &shader_src_ptr, nullptr);
        glCompileShader(shader_id);
    
        ErrorInt success(Shader::SHADER_COMPILE_FAIL);
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, success.v_ptr());
        return success;
    }
    
    static ErrorInt create_program(
        GLuint& p_id,
        const GLuint vs_id,
        const GLuint fs_id
    ) {
        p_id = glCreateProgram();
        glAttachShader(p_id, vs_id);
        glAttachShader(p_id, fs_id);
        glLinkProgram(p_id);
    
        ErrorInt success(Shader::PROGRAM_LINK_FAIL);
        glGetProgramiv(p_id, GL_LINK_STATUS, success.v_ptr());
        return success;
    }
    
    std::optional<std::reference_wrapper<Shader>> init_program(
        const std::string& vert_shader_src,
        const std::string& frag_shader_src
    ) {
        char infoLog[512];
        GLuint v_shader_id, f_shader_id;
        if (!Shader::create_shader(v_shader_id, vert_shader_src, GL_VERTEX_SHADER)) {
            glGetShaderInfoLog(v_shader_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            return std::nullopt;
        }
    
        if (!Shader::create_shader(f_shader_id, frag_shader_src, GL_FRAGMENT_SHADER)) {
            glGetShaderInfoLog(f_shader_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            return std::nullopt;
        }
    
        if (!Shader::create_program(this->program_id, v_shader_id, f_shader_id)) {
            glGetProgramInfoLog(this->program_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            return std::nullopt;
        }
    
        glDeleteShader(v_shader_id);
        glDeleteShader(f_shader_id);
        return std::ref(*this);
    }
public:

    static std::optional<Shader> from_string(const std::string& vs, const std::string& fs) {
        std::optional<Shader> s = Shader();
        return s->init_program(vs, fs);
    }

    static std::optional<std::string> read_file(const std::string& fpath) {
        std::ifstream file(fpath);
        if (!file.is_open()) {
            return std::nullopt;
        }

        std::stringstream buff;
        buff << file.rdbuf();
        return buff.str();
    }

    static std::optional<Shader> from_file(const std::string& vs_path, const std::string& fs_path) {
        std::optional<Shader> s = Shader();
        std::optional<std::string> vs = Shader::read_file(vs_path);
        std::optional<std::string> fs = Shader::read_file(fs_path);
        return vs && fs ? s->init_program(*vs, *fs) : std::nullopt;
    }

    Shader& use() {
        glUseProgram(this->program_id);
        return *this;
    }

    Shader& add_uniform(const std::string& uniform_name, std::function<void(GLuint program, GLint location)> F) {
        uniform_map::iterator it = this->umap.find(uniform_name);
        const GLint location = it == this->umap.end()
            ? glGetUniformLocation(this->program_id, uniform_name.c_str())
            : it->second.location;

        this->umap[uniform_name] = Uniform {
            location,
            F
        };

        return *this;
    }

    // call all uniform functions
    Shader& ping_all_uniforms() {
        GLint prev_program_id = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program_id);
        glUseProgram(this->program_id);

        for (std::pair<std::string, Uniform> p : this->umap) {
            p.second.func(this->program_id, p.second.location);
        }

        glUseProgram(prev_program_id);
        return *this;
    }

    // call a specific uniform function
    Shader& ping_uniform(const std::string& uniform_name) {
        GLint prev_program_id = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program_id);
        glUseProgram(this->program_id);

        uniform_map::iterator it = this->umap.find(uniform_name);
        if (it == this->umap.end()) {
            std::cout << "Uniform \"" << uniform_name << "\" could not be found..." << std::endl;
        } else {
            it->second.func(this->program_id, it->second.location);
        }

        glUseProgram(prev_program_id);
        return *this;
    }

    GLuint get_program_id() const {
        return this->program_id;
    }
};
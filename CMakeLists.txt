# cmake is hell
cmake_minimum_required(VERSION 3.5)
project(metaballs)

# c++ ver
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DEP_DIR src/dependencies)

# adding glfw
add_subdirectory(${DEP_DIR}/glfw-3.4)

# finding opengl
find_package(OpenGL REQUIRED)

add_library(glad STATIC ${DEP_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${DEP_DIR}/glad/include)

# add executables
add_executable(metaballs src/main.cpp)

target_include_directories(metaballs PRIVATE ${DEP_DIR}/glad/include)

# link against both opengl & glfw
target_link_libraries(metaballs PRIVATE glad glfw OpenGL::GL)
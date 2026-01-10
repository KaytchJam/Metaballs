# cmake is hell
cmake_minimum_required(VERSION 3.5)
project(metaballs)

# c++ ver
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DEP_DIR src/dependencies)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# adding glfw
add_subdirectory(${DEP_DIR}/glfw-3.4)

# finding opengl
find_package(OpenGL REQUIRED)

add_library(glad STATIC ${DEP_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${DEP_DIR}/glad/include)

# add executables
add_executable(metaballs src/main.cpp src/fieldrange.cpp src/intrange.cpp src/isosurface.cpp src/marcher.cpp)
add_executable(lt src/lalg_test.cpp)

target_include_directories(metaballs PRIVATE src/include)
target_include_directories(metaballs PRIVATE ${DEP_DIR}/glad/include)
target_include_directories(metaballs PRIVATE ${DEP_DIR})

target_include_directories(lt PRIVATE src/include)
target_include_directories(lt PRIVATE ${DEP_DIR})

# link against both opengl & glfw
target_link_libraries(metaballs PRIVATE glad glfw OpenGL::GL)
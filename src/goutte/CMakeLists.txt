cmake_minimum_required(VERSION 3.5)
project(goutte)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(goutte STATIC)

target_sources(goutte PRIVATE src/fieldrange.cpp src/intrange.cpp src/isosurface.cpp src/marcher.cpp src/dsa/unionfind.cpp src/boundingbox.cpp src/aabb_tree.cpp)

target_include_directories(goutte PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# if (BUILD_TESTING)
    add_executable(lt tests/lalg_test.cpp) # tests the vector algebra header file
    add_executable(st tests/surface_test.cpp) # tests the isosurface
    add_executable(bm tests/benchmark.cpp)

    # setting up include paths
    target_include_directories(lt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(st PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(bm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # linking
    target_link_libraries(st PRIVATE goutte)
    target_link_libraries(bm PRIVATE goutte)

    # add_test(NAME LalgTest COMMAND lt)
    # add_test(NAME SurfaceTest COMMAND st)
    # add_test(NAME BenchmarkTest COMMAND bm)
# endif()

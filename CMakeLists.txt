cmake_minimum_required(VERSION 4.4)
project(boids)
find_package(SDL3 REQUIRED)
add_executable(boids
    src/main.cpp
)
target_link_libraries(boids PRIVATE SDL3::SDL3)
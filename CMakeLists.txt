cmake_minimum_required(VERSION 3.20)
project(boids 
  VERSION 1.0
  LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(boids
  src/main.cpp
)

find_package(SDL3)
target_link_libraries(boids PRIVATE SDL3::SDL3)

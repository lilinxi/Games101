cmake_minimum_required(VERSION 3.15)
project(Delaunay)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
include_directories(${SFML_INCLUDE_DIRS})

add_executable(Delaunay main.cpp delaunay.cpp edge.cpp triangle.cpp vector2.cpp)
target_link_libraries(Delaunay sfml-graphics)
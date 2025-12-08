cmake_minimum_required(VERSION 3.15)

project(MyRaylibGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the raylib submodule directory.
add_subdirectory(external/raylib)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "src/*.h")

add_executable(main ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(main PUBLIC src)

target_link_libraries(main PRIVATE raylib)

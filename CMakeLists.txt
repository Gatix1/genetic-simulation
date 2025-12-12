cmake_minimum_required(VERSION 3.15)

project(MyRaylibGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the raylib submodule directory.
add_subdirectory(external/raylib)

# ImGui and rlImGui setup
set(IMGUI_DIR "external/imgui")
set(RLIMGUI_DIR "external/rlImGui")

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "src/*.h")
file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp" "${IMGUI_DIR}/*.h")
file(GLOB RLIMGUI_SOURCES "${RLIMGUI_DIR}/*.cpp" "${RLIMGUI_DIR}/*.h")

add_executable(main ${SOURCE_FILES} ${HEADER_FILES} ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

target_include_directories(main PUBLIC src ${IMGUI_DIR} ${RLIMGUI_DIR})

target_link_libraries(main PRIVATE raylib)

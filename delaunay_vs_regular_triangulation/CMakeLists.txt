cmake_minimum_required(VERSION 3.16)
project(DT_vs_RT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED COMPONENTS Core)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# ./libs/imgui repo from git
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)


# fetch GLAD for convenience 
include(FetchContent)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

add_executable(dt_vs_rt
    main.cpp
    ${IMGUI_SRC}
)

target_include_directories(dt_vs_rt PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(dt_vs_rt
    PRIVATE
    CGAL::CGAL
    OpenGL::GL
    glfw
    glad
)

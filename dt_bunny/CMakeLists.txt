cmake_minimum_required(VERSION 3.12)
project(DT_Bunny)

set(CMAKE_CXX_STANDARD 17)

find_package(CGAL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

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

set(ALL_LIBS 
    CGAL::CGAL
    OpenGL::GL
    glfw
    glad
)

# fetch GLAD for convenience 
include(FetchContent)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/src
)
add_executable(dt_bunny
    dt_bunny.cpp
    ${IMGUI_SRC}
)

target_include_directories(dt_bunny PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(dt_bunny PRIVATE ${ALL_LIBS})


cmake_minimum_required(VERSION 3.12)
project(BunnyViewer)

set(CMAKE_CXX_STANDARD 17)

find_package(CGAL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# need EIGEN library for Poisson Surface Reconstruction
# https://stackoverflow.com/questions/31547122/surface-mesh-generation-code-in-cgal-not-compiling
find_package(Eigen3 REQUIRED)
  if (EIGEN3_FOUND) 
    include( ${EIGEN3_USE_FILE} )
endif()


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

# # poisson-surface for stanford bunny
# add_executable(poisson_bunny poisson_bunny.cpp)


# Set include directories for Boost and CGAL if youre building Boost,CGAL from source
set(BOOST_INCLUDE_DIR /home/nmohan/Desktop/test/boost_1_89_0/)
set(CGAL_INCLUDE_DIR /home/nmohan/Desktop/test/cgal-example/CGAL-6.1/include)
set(BOOST_LIBRARY_DIR /home/nmohan/Desktop/test/boost_1_89_0/libs)
include_directories(${BOOST_INCLUDE_DIR} ${CGAL_INCLUDE_DIR})
link_directories( ${BOOST_LIBRARY_DIR} )
link_libraries("gmp")
add_executable(poisson poisson.cpp)
target_link_libraries(poisson PRIVATE CGAL::CGAL)


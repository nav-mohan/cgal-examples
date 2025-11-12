
# # If you are building BOOST and CGAL from source...
# set(BOOST_INCLUDE_DIR /home/nmohan/Desktop/test/boost_1_89_0/)
# set(CGAL_INCLUDE_DIR /home/nmohan/Desktop/test/cgal-example/CGAL-6.1/include)
# set(BOOST_LIBRARY_DIR /home/nmohan/Desktop/test/boost_1_89_0/libs)
# include_directories(${BOOST_INCLUDE_DIR} ${CGAL_INCLUDE_DIR} )
# link_directories(${BOOST_LIBRARY_DIR})
# link_libraries("gmp")

cmake_minimum_required(VERSION 3.16)
project(voronoi_delaunay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED COMPONENTS Core)
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

add_executable(voronoi_delaunay
    main.cpp
    ${IMGUI_SRC}
)

target_include_directories(voronoi_delaunay PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(voronoi_delaunay PRIVATE ${ALL_LIBS})

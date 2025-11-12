#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <CGAL/squared_distance_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Alpha_shape_cell_base_3.h>
#include <CGAL/Triangulation_data_structure_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Alpha_shape_vertex_base_3<K> Vb;
typedef CGAL::Alpha_shape_cell_base_3<K> Cb;
typedef CGAL::Triangulation_data_structure_3<Vb, Cb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds> Delaunay;
typedef CGAL::Alpha_shape_3<Delaunay> Alpha_shape_3;
// typedef CGAL::Alpha_shape_3<Delaunay, CGAL::Tag_true> Alpha_shape_3;
typedef K::Point_3 Point;


std::vector<Point> load_csv(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Point> pts;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        float x, y, z;
        char comma;
        if (ss >> x >> comma >> y >> comma >> z) pts.push_back({x, y, z});
    }
    return pts;
}

std::vector<Point> build_alpha_edges(const std::vector<Point>& pts, double alpha) {
    std::vector<Point> lines;
    if (pts.empty()) return lines;

    Alpha_shape_3 A(pts.begin(), pts.end(), alpha, Alpha_shape_3::GENERAL);

    for (auto it = A.alpha_shape_facets_begin(); it != A.alpha_shape_facets_end(); ++it) {
        
        auto cell = it->first;
        int i = it->second;

        // Collect the 3 vertices of this facet
        Point pts[3];
        int idx = 0;
        for (int j = 0; j < 4; ++j)
        {
            if(j == i) continue;
            pts[idx] = cell->vertex(j)->point();
            idx++;
        }

        // Create and store 3 edges of the triangle
        auto push_edge = [&](const Point& a, const Point& b) {
            lines.push_back(a);
            lines.push_back(b);
        };

        push_edge(pts[0], pts[1]);
        push_edge(pts[1], pts[2]);
        push_edge(pts[2], pts[0]);

    }

    std::cout << "Alpha: " << alpha << " → edges: " << lines.size() / 2 << std::endl;
    return lines;
}

// Normalize to unit cube
std::vector<Point> normalize_points(std::vector<Point>& pts) {
    if (pts.empty()) return {};
    float minx=1e9, miny=1e9, minz=1e9;
    float maxx=-1e9, maxy=-1e9, maxz=-1e9;
    for (auto& p : pts) {
        const float x = p.x();
        const float y = p.y();
        const float z = p.z();

        minx = std::min(minx, x);
        miny = std::min(miny, y);
        minz = std::min(minz, z);
        maxx = std::max(maxx, x);
        maxy = std::max(maxy, y);
        maxz = std::max(maxz, z);
    }
    float cx = (minx + maxx) / 2.0f;
    float cy = (miny + maxy) / 2.0f;
    float cz = (minz + maxz) / 2.0f;
    float scale = std::max({maxx - minx, maxy - miny, maxz - minz});
    
    std::vector<Point> norm_pts;
    for (auto& p : pts) {
        const float x = (p.x() - cx) / scale;
        const float y = (p.y() - cy) / scale;
        const float z = (p.z() - cz) / scale;
        norm_pts.push_back({x,y,z});
    }
    return norm_pts;
}

// Render helpers
void draw_lines(const std::vector<Point>& lines) {
    glBegin(GL_LINES);
    for (auto& p : lines) glVertex3f(p.x(), p.y(), p.z());
    glEnd();
}

void draw_points(const std::vector<Point>& pts) {
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (auto& p : pts) glVertex3f(p.x(), p.y(), p.z());
    glEnd();
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "CGAL Alpha Shape Viewer", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    std::vector<Point> data = load_csv("../bunny.csv");
    float alpha = 0.05f;
    bool show_alpha = false;
    bool recompute = true;
    auto points = std::move(normalize_points(data));

    std::vector<Point> alpha_edges;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        if (recompute) {
            alpha_edges = build_alpha_edges(points, alpha);
            recompute = false;
        }

        if (show_alpha)
            draw_lines(alpha_edges);
        else
            draw_points(points);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Alpha Shape Controls");
        ImGui::Checkbox("Show Alpha Shape", &show_alpha);
        if (ImGui::SliderFloat("Alpha", &alpha, 0.001f, 0.02f)) {
            recompute = true;
        }
        ImGui::Text("Points: %zu", points.size());
        ImGui::Text("Edges: %zu", alpha_edges.size() / 2);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

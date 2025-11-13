#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>

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

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel_t;
typedef CGAL::Delaunay_triangulation_3<Kernel_t> Delaunay_t;
typedef Kernel_t::Point_3 Point3;

struct Point{float x, y, z;};

std::vector<Point> LoadCSV(const std::string &filename)
{
    std::vector<Point> pts;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << filename << "\n";
        return pts;
    }
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string sx, sy, sz;
        std::getline(ss, sx, ',');
        std::getline(ss, sy, ',');
        std::getline(ss, sz, ',');
        pts.push_back({std::stof(sx), std::stof(sy), std::stof(sz)});
    }
    std::cout << "Loaded " << pts.size() << " points\n";
    return pts;
}

// normalize poitns to unit cube
void NormalizePoints(std::vector<Point> &pts)
{
    if (pts.empty()) return;
    float minx = 1e9, miny = 1e9, minz = 1e9;
    float maxx = -1e9, maxy = -1e9, maxz = -1e9;
    for (auto &p : pts)
    {
        minx = std::min(minx, p.x);
        miny = std::min(miny, p.y);
        minz = std::min(minz, p.z);
        maxx = std::max(maxx, p.x);
        maxy = std::max(maxy, p.y);
        maxz = std::max(maxz, p.z);
    }
    float cx = (minx + maxx) / 2.0f;
    float cy = (miny + maxy) / 2.0f;
    float cz = (minz + maxz) / 2.0f;
    float scale = std::max({maxx - minx, maxy - miny, maxz - minz});
    for (auto &p : pts)
    {
        p.x = (p.x - cx) / scale;
        p.y = (p.y - cy) / scale;
        p.z = (p.z - cz) / scale;
    }
}

// global variables for edge cutoffs
float max_edge_length = 0.2f;
bool show_delaunay = false;
bool recompute_edges = false;

// Build Delaunay and return edges as line segments
std::vector<Point> BuildDelaunayEdges(const std::vector<Point> &pts, float max_length = -1.0f)
{
    if (pts.empty()) return {};

    std::vector<Point> lines;

    std::vector<Point3> cgal_points;
    for (auto &p : pts) cgal_points.emplace_back(p.x, p.y, p.z);

    Delaunay_t T;
    T.insert(cgal_points.begin(), cgal_points.end());
    std::cout << "vertices: " << T.number_of_vertices() << "\n";
    std::cout << "edges: " << T.number_of_finite_edges() << "\n";

    for (auto eit = T.finite_edges_begin(); eit != T.finite_edges_end(); ++eit)
    {
        auto seg = T.segment(*eit);
        double len = std::sqrt(CGAL::squared_distance(seg.source(), seg.target()));
        if (max_length > 0.0f && len > max_length) continue;
        lines.push_back({(float)seg.source().x(), (float)seg.source().y(), (float)seg.source().z()});
        lines.push_back({(float)seg.target().x(), (float)seg.target().y(), (float)seg.target().z()});
    }
    std::cout << "generated " << lines.size() / 2 << " edges.\n";
    return lines;
}

void DrawPoints(const std::vector<Point> &pts)
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.8f, 0.1f);
    for (auto &p : pts) glVertex3f(p.x, p.y, p.z);
    glEnd();
}

void DrawLines(const std::vector<Point> &lines)
{
    glBegin(GL_LINES);
    glColor3f(0.1f, 0.9f, 1.0f);
    for (auto &p : lines) glVertex3f(p.x, p.y, p.z);
    glEnd();
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(800, 600, "delaunay triangulation stanford bunny", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    // load points and normalize
    std::vector<Point> points = LoadCSV("../bunny.csv");
    NormalizePoints(points);

    // perform triangulation
    std::vector<Point> delaunay_edges = BuildDelaunayEdges(points, 0.2f);

    bool show_delaunay = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)width / (float)height;
        glFrustum(-aspect, aspect, -1, 1, 1.5, 10.0);

        // transform
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -2.0f);

        DrawPoints(points);
        if (show_delaunay)
            DrawLines(delaunay_edges);

        // ImGui UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Checkbox("show triangulation", &show_delaunay);

        if (ImGui::SliderFloat("max edge length", &max_edge_length, 0.0f, 0.3f))
            recompute_edges = true;

        if (recompute_edges)
        {
            delaunay_edges = BuildDelaunayEdges(points, max_edge_length);
            recompute_edges = false;
        }

        ImGui::Text("#points: %zu", points.size());
        ImGui::Text("#edges: %zu", delaunay_edges.size() / 2);

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

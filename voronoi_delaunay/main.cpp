#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Voronoi_diagram_2.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <vector>
#include <cmath>
#include <iostream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;

static Delaunay dt;
static std::vector<Point> points;

static int window_width = 900, window_height = 600;
static bool show_delaunay = true;
static bool show_voronoi = true;
static bool show_points = true;

Point screen_to_world(double xpos, double ypos) {
    double x = xpos / window_width;
    double y = 1.0 - ypos / window_height;
    return Point(x, y);
}

void recompute() {
    dt.clear();
    if (!points.empty())
        dt.insert(points.begin(), points.end());
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    Point p = screen_to_world(xpos, ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        points.push_back(p);
        recompute();
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // delete nearest point (if close)
        if (points.empty()) return;
        double min_dist = 0.02;
        auto nearest = points.end();
        for (auto it = points.begin(); it != points.end(); ++it) {
            double d = std::sqrt(CGAL::squared_distance(*it, p));
            if (d < min_dist) {
                min_dist = d;
                nearest = it;
            }
        }
        if (nearest != points.end()) {
            points.erase(nearest);
            recompute();
        }
    }
}

void draw_delaunay(const Delaunay& dt) {
    glColor3f(0.2f, 0.6f, 1.0f);
    glBegin(GL_LINES);
    for (auto f = dt.finite_faces_begin(); f != dt.finite_faces_end(); ++f) {
        for (int i = 0; i < 3; ++i) {
            Point a = f->vertex(i)->point();
            Point b = f->vertex((i + 1) % 3)->point();
            glVertex2f(a.x(), a.y());
            glVertex2f(b.x(), b.y());
        }
    }
    glEnd();
}

void draw_voronoi(const Delaunay& dt) {
    glColor3f(1.0f, 0.85f, 0.1f);
    glBegin(GL_LINES);
    for (auto e = dt.finite_edges_begin(); e != dt.finite_edges_end(); ++e) {
        CGAL::Object o = dt.dual(e);
        if (const K::Segment_2* s = CGAL::object_cast<K::Segment_2>(&o)) {
            glVertex2f(s->source().x(), s->source().y());
            glVertex2f(s->target().x(), s->target().y());
        } else if (const K::Ray_2* r = CGAL::object_cast<K::Ray_2>(&o)) {
            // Clip to a box [0,1]^2 for visualization
            K::Point_2 src = r->source();
            K::Point_2 dir = src + 0.5 * (r->to_vector() / std::sqrt(r->to_vector().squared_length()));
            glVertex2f(src.x(), src.y());
            glVertex2f(dir.x(), dir.y());
        }
    }
    glEnd();
}

void draw_points() {
    glPointSize(6.0f);
    glColor3f(1.0f, 0.3f, 0.3f);
    glBegin(GL_POINTS);
    for (auto& p : points)
        glVertex2f(p.x(), p.y());
    glEnd();
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Delaunay + Voronoi Visualization", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    recompute();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- ImGui UI ---
        ImGui::Begin("Controls");
        ImGui::Text("Left click: add point");
        ImGui::Text("Right click: delete point");
        ImGui::Separator();
        ImGui::Checkbox("Show Delaunay", &show_delaunay);
        ImGui::Checkbox("Show Voronoi", &show_voronoi);
        ImGui::Checkbox("Show Points", &show_points);
        if (ImGui::Button("Clear All")) {
            points.clear();
            recompute();
        }
        ImGui::End();

        // --- OpenGL rendering ---
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (show_delaunay) draw_delaunay(dt);
        if (show_voronoi) draw_voronoi(dt);
        if (show_points) draw_points();

        // Render ImGui overlay
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

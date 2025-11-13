#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Regular_triangulation_2.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <vector>
#include <iostream>
#include <cstdlib>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel_t;
typedef CGAL::Delaunay_triangulation_2<Kernel_t> Delaunay_t;
typedef CGAL::Regular_triangulation_2<Kernel_t> Regular_t;
typedef Kernel_t::Point_2 Point;
typedef Kernel_t::Weighted_point_2 WeightedPoint;

// global variables!
static Delaunay_t delaunay;
static Regular_t regular;
static std::vector<Point> points;
static std::vector<WeightedPoint> weighted_points;
static int window_width = 1000;
static int window_height = 500;

Point ScreenToWorld(double xpos, double ypos, double x_offset = 0.0)
{
    double x = (xpos - x_offset) / (window_width / 2.0);
    double y = 1.0 - ypos / window_height;
    return Point(x, y);
}

void Recompute()
{
    delaunay.clear();
    regular.clear();
    if (!points.empty())
    {
        delaunay.insert(points.begin(), points.end());
        regular.insert(weighted_points.begin(), weighted_points.end());
    }
}

// callback funtion for mouse clicks
void MouseBtnCB(GLFWwindow *window, int button, int action, int mods)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) return; // ImGUI has captured the mouse click

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Determine which side was clicked
        if (xpos < window_width / 2.0)
        {
            Point p = ScreenToWorld(xpos, ypos, 0.0);
            points.push_back(p);
            weighted_points.push_back(WeightedPoint(p, 0.0));
            Recompute();
        }
    }
}

void DrawTriangulation(const Delaunay_t &dt, double x_offset)
{
    glPushMatrix();
    glTranslated(x_offset, 0, 0);
    glColor3f(1.0f, 1.0f, 1.0f);
    // glColor3f(0.2f, 0.6f, 1.0f);
    glBegin(GL_LINES);
    for (auto f = dt.finite_faces_begin(); f != dt.finite_faces_end(); ++f)
    {
        auto tri = dt.triangle(f);
        for (int i = 0; i < 3; ++i)
        {
            auto a = tri.vertex(i);
            auto b = tri.vertex((i + 1) % 3);
            glVertex2f(a.x(), a.y());
            glVertex2f(b.x(), b.y());
        }
    }
    glEnd();
    glPopMatrix();
}

void DrawTriangulation(const Regular_t &rt, double x_offset)
{
    glPushMatrix();
    glTranslated(x_offset, 0, 0);
    glColor3f(1.0f, 1.0f, 1.0f);
    // glColor3f(0.3f, 0.9f, 0.3f);
    glBegin(GL_LINES);
    for (auto f = rt.finite_faces_begin(); f != rt.finite_faces_end(); ++f)
    {
        auto tri = rt.triangle(f);
        for (int i = 0; i < 3; ++i)
        {
            auto a = tri.vertex(i);
            auto b = tri.vertex((i + 1) % 3);
            glVertex2f(a.x(), a.y());
            glVertex2f(b.x(), b.y());
        }
    }
    glEnd();
    glPopMatrix();
}

void DrawPoints(double x_offset)
{
    glPushMatrix();
    glTranslated(x_offset, 0, 0);
    glPointSize(6.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    // glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_POINTS);
    for (auto &p : points)
        glVertex2f(p.x(), p.y());
    glEnd();
    glPopMatrix();
}

// label each vertex with it's weights
void RenderWeightsTextOverlay(const std::vector<WeightedPoint> &wpoints, double x_offset, int window_width, int window_height)
{
    ImDrawList *draw_list = ImGui::GetForegroundDrawList();
    ImU32 col = IM_COL32(255, 255, 255, 255); 

    for (auto &wp : wpoints)
    {
        double x = wp.x() + x_offset; // offset for right half
        double y = wp.y();

        // Convert from world to screen 
        float sx = (float)(x / 2.0 * window_width);
        float sy = (float)((1.0 - y) * window_height);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", wp.weight());
        draw_list->AddText(ImVec2(sx + 5, sy - 10), col, buf);
    }
}

int main()
{
    // GLFW and OpenGL setup
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "DT vs RT", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, MouseBtnCB);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initial points
    points = {{0.2, 0.2}, {0.8, 0.2}, {0.5, 0.8}};
    weighted_points = {
        WeightedPoint(points[0], 0.1),
        WeightedPoint(points[1], 0.2),
        WeightedPoint(points[2], 0.3)};
    Recompute();

    bool show_points = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Text("Click on left side to add points");
        if (ImGui::Button("clear all points"))
        {
            points.clear();
            weighted_points.clear();
            Recompute();
        }
        if (ImGui::Button("randomize weights"))
        {
            weighted_points.clear();
            for (auto &p : points)
                weighted_points.push_back(WeightedPoint(p, (rand() % 100) / 300.0));
            Recompute();
        }

        glViewport(0, 0, window_width, window_height);
        // glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, 2.0, 0.0, 1.0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // render DT in left panel
        DrawTriangulation(delaunay, 0.0);
        DrawPoints(0.0);

        // render RT in right panel
        DrawTriangulation(regular, 1.0);
        DrawPoints(1.0);

        // render weights text label in right panel
        RenderWeightsTextOverlay(weighted_points, 1.0, window_width, window_height);

        // Center line
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(1.0, 0.0);
        glVertex2f(1.0, 1.0);
        glEnd();

        // ImGui render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

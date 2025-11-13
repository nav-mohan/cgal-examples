#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/compute_average_spacing.h>
#include <CGAL/pca_estimate_normals.h>
#include <CGAL/mst_orient_normals.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_points.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <utility>
#include <list>
#include <fstream>

// Types
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel_t;
typedef Kernel_t::Point_3 Point;
typedef Kernel_t::Vector_3 Vector;
typedef std::pair<Point, Vector> PointVectorPair; // Point with normal vector stored in a std::pair.

std::vector<Point> NormalizePoints(std::vector<Point> &pts)
{
  if (pts.empty()) return {};

  float minx = 1e9, miny = 1e9, minz = 1e9;
  float maxx = -1e9, maxy = -1e9, maxz = -1e9;
  
  for (auto &p : pts)
  {
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
  for (auto &p : pts)
  {
    const float x = (p.x() - cx) / scale;
    const float y = (p.y() - cy) / scale;
    const float z = (p.z() - cz) / scale;
    norm_pts.push_back({x, y, z});
  }
  return norm_pts;
}

void RenderPointsWithNormals(const std::vector<Point> &points, const std::vector<Vector> &normals, float normal_scale = 0.05f)
{
  // Draw points
  glPointSize(4.0f);
  glBegin(GL_POINTS);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (const auto &p : points)
    glVertex3f(p.x(), p.y(), p.z());
  glEnd();

  // Draw normals as lines
  glBegin(GL_LINES);
  glColor3f(1.0f, 0.2f, 0.2f);
  for (size_t i = 0; i < points.size(); ++i)
  {
    const Point &p = points[i];
    const Vector &n = normals[i];

    glVertex3f(p.x(), p.y(), p.z());
    glVertex3f(p.x() + n.x() * normal_scale,
               p.y() + n.y() * normal_scale,
               p.z() + n.z() * normal_scale);
  }
  glEnd();
}

float camera_theta = 0.0f; // horizontal angle
float camera_phi = 0.0f;   // vertical angle
float camera_distance = 1.0f;

double last_x, last_y;
bool rotating = false;
bool zooming = false;

// callback for mouse clicks
void MouseBtnCB(GLFWwindow *window, int button, int action, int mods)
{
  if (ImGui::GetIO().WantCaptureMouse) return; // ImGui captured mouseclick - ignore

  if (button == GLFW_MOUSE_BUTTON_LEFT)
  {
    rotating = (action == GLFW_PRESS);
    glfwGetCursorPos(window, &last_x, &last_y);
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT)
  {
    zooming = (action == GLFW_PRESS);
    glfwGetCursorPos(window, &last_x, &last_y);
  }
}

// mouse callback for panning
void CursorDragCB(GLFWwindow *window, double xpos, double ypos)
{
  if (rotating)
  {
    float dx = float(xpos - last_x);
    float dy = float(ypos - last_y);
    last_x = xpos;
    last_y = ypos;

    camera_theta -= dx * 0.005f;
    camera_phi += dy * 0.005f;

    // restrict phi to avoid flipping 
    if (camera_phi > 1.5f) camera_phi = 1.5f;
    if (camera_phi < -1.5f) camera_phi = -1.5f;
  }

  if (zooming)
  {
    float dy = float(ypos - last_y);
    last_y = ypos;
    camera_distance *= (1.0f + dy * 0.01f);
    
    // dont zoom beyond [0.1,10.0]
    if (camera_distance < 0.1f) camera_distance = 0.1f;
    if (camera_distance > 10.0f) camera_distance = 10.0f;
  }
}

// scroll callback function - do zooming
void ScrollCB(GLFWwindow *window, double xoffset, double yoffset)
{
  camera_distance *= (1.0 - yoffset * 0.1f);

  // dont zoom beyond [0.1,10.0]
  if (camera_distance < 0.1f) camera_distance = 0.1f;
  if (camera_distance > 10.0f) camera_distance = 10.0f;
}

// compute projection matrix for rotation and translation(zoom)
void SetupViewport(int width, int height)
{
  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // projection
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.01f, 100.0f);

  // camera position and angle
  float camX = camera_distance * cos(camera_phi) * sin(camera_theta);
  float camY = camera_distance * sin(camera_phi);
  float camZ = camera_distance * cos(camera_phi) * cos(camera_theta);
  glm::vec3 eye(camX, camY, camZ);
  glm::vec3 center(0.0f);
  glm::vec3 up(0.0f, 1.0f, 0.0f);
  glm::mat4 view = glm::lookAt(eye, center, up);

  // apply projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(&projection[0][0]);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(&view[0][0]);
}

int main(int argc, char *argv[])
{
  if (!glfwInit()) return -1;

  GLFWwindow *window = glfwCreateWindow(800, 600, "normals bunny", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSetMouseButtonCallback(window, MouseBtnCB);
  glfwSetCursorPosCallback(window, CursorDragCB);
  glfwSetScrollCallback(window, ScrollCB);
  glEnable(GL_DEPTH_TEST); // Enable depth for 3D 

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  ImGui::StyleColorsDark();

  const std::string fname = "../bunny.xyz";

  // Reads a point set file in points[].
  std::list<PointVectorPair> points;
  if (!CGAL::IO::read_points(fname,
                             std::back_inserter(points),
                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>())))
  {
    std::cerr << "Error: cannot read file " << fname << std::endl;
    return EXIT_FAILURE;
  }

  // Estimates normals direction.
  // Note: pca_estimate_normals() requiresa range of points as well as property maps to access each point's position and normal.
  const int nb_neighbors = 18; // K-nearest neighbors = 3 rings

  CGAL::pca_estimate_normals<CGAL::Parallel_if_available_tag>
      (points, nb_neighbors,
       CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>())
           .normal_map(CGAL::Second_of_pair_property_map<PointVectorPair>()));

  // Orients normals.
  // Note: mst_orient_normals() requires a range of points as well as property maps to access each point's position and normal.
  std::list<PointVectorPair>::iterator unoriented_points_begin =
      CGAL::mst_orient_normals(points, nb_neighbors,
                               CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>())
                                   .normal_map(CGAL::Second_of_pair_property_map<PointVectorPair>()));

  // Optional: delete points with an unoriented normal
  // if you plan to call a reconstruction algorithm that expects oriented normals.
  points.erase(unoriented_points_begin, points.end());

  std::vector<Point> pointsToVisualize;
  std::vector<Vector> normalsToVisualize;
  std::ofstream outfile("../bunny_with_normals.xyz");
  for (auto p : points)
  {
    auto pt = p.first;
    auto vec = p.second;

    pointsToVisualize.emplace_back(pt.x(), pt.y(), pt.z());
    normalsToVisualize.emplace_back(vec.x(), vec.y(), vec.z());

    const float x = pt.x();
    const float nx = vec.x();
    const float y = pt.y();
    const float ny = vec.y();
    const float z = pt.z();
    const float nz = vec.z();

    outfile << x << " " << nx << " " << y << " " << ny << " " << z << " " << nz << "\n";
  }

  pointsToVisualize = std::move(NormalizePoints(pointsToVisualize));

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    SetupViewport(800, 600);
    RenderPointsWithNormals(pointsToVisualize, normalsToVisualize);

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
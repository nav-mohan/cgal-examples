#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

struct Mesh
{
    std::vector<float> vertices; // x,y,z
    std::vector<unsigned int> indices;
};

bool LoadOFF(const std::string &filename, Mesh &mesh)
{
    std::ifstream in(filename);
    if (!in)
    {
        std::cerr << "Failed to open OFF file\n";
        return false;
    }

    std::string header;
    in >> header;
    if (header != "OFF")
    {
        std::cerr << "Invalid OFF file\n";
        return false;
    }

    size_t n_vertices, n_faces, n_edges;
    in >> n_vertices >> n_faces >> n_edges;

    mesh.vertices.resize(n_vertices * 3);
    for (size_t i = 0; i < n_vertices; ++i)
        in >> mesh.vertices[3 * i] >> mesh.vertices[3 * i + 1] >> mesh.vertices[3 * i + 2];

    for (size_t i = 0; i < n_faces; ++i)
    {
        int n, v0, v1, v2;
        in >> n >> v0 >> v1 >> v2;
        if (n != 3) continue; // only triangles
        mesh.indices.push_back(v0);
        mesh.indices.push_back(v1);
        mesh.indices.push_back(v2);
    }

    return true;
}

GLuint createShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) printf("SHADER ERROR\n");
    return shader;
}

GLuint createProgram(const char *vs, const char *fs)
{
    GLuint v = createShader(GL_VERTEX_SHADER, vs);
    GLuint f = createShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

const char *vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 MVP;
void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char *fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.7, 0.7, 0.9, 1.0);
}
)";

// global variables for zoom, rotate
float yaw = 0.0f, pitch = 0.0f;
float distance = 3.0f;
double lastX, lastY;
bool dragging = false;

void MouseBtnCB(GLFWwindow *window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    
    if (action == GLFW_PRESS)
    {
        dragging = true;
        glfwGetCursorPos(window, &lastX, &lastY);
    }
    else if (action == GLFW_RELEASE) dragging = false;
}

void CursorDragCB(GLFWwindow *window, double xpos, double ypos)
{
    if(!dragging) return;
    float dx = xpos - lastX;
    float dy = ypos - lastY;
    yaw += dx * 0.3f;
    pitch += dy * 0.3f;
    lastX = xpos;
    lastY = ypos;
}

void ScrollCB(GLFWwindow *window, double xoff, double yoff)
{
    distance -= yoff * 0.1f;
    if (distance < 0.5f) distance = 0.5f;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "OFF viewer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetMouseButtonCallback(window, MouseBtnCB);
    glfwSetCursorPosCallback(window, CursorDragCB);
    glfwSetScrollCallback(window, ScrollCB);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    Mesh mesh;
    if (!LoadOFF("../bunny.off", mesh)) return -1;
    std::cout << "loaded vertices: " << mesh.vertices.size() / 3 << " faces: " << mesh.indices.size() / 3 << "\n";

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint program = createProgram(vertexShaderSrc, fragmentShaderSrc);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1, 0.1, 0.12, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -distance));
        view = glm::rotate(view, glm::radians(pitch), glm::vec3(1, 0, 0));
        view = glm::rotate(view, glm::radians(yaw), glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), float(w) / h, 0.01f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;
        glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

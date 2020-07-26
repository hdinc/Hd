#include <Hd.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "circle.h"
#include <stdio.h>

class grid {
public:
    GLuint vao, vbo;
    int c = 65;
    Hd::Shader* shader;
    glm::mat4* view;
    glm::mat4* projection;
    GLint COLOR, MVP;
    grid(Hd::Shader* s, glm::mat4* v, glm::mat4* p)
    {

        shader = s;
        view = v;
        projection = p;

        COLOR = glGetUniformLocation(shader->Id(), "COLOR");
        MVP = glGetUniformLocation(shader->Id(), "MVP");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        float g[c][2][2][2];

        for (int i = 0; i < c; i++) {
            g[i][0][0][0] = -c;
            g[i][0][0][1] = i - (c - 1) / 2.0f;
            g[i][0][1][0] = +c;
            g[i][0][1][1] = i - (c - 1) / 2.0f;
        }

        for (int i = 0; i < c; i++) {
            g[i][1][0][1] = -c;
            g[i][1][0][0] = i - (c - 1) / 2.0f;
            g[i][1][1][1] = +c;
            g[i][1][1][0] = i - (c - 1) / 2.0f;
        }
        glBufferData(
            GL_ARRAY_BUFFER, c * 2 * 2 * 2 * sizeof(float), g, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    void draw()
    {
        glBindVertexArray(vao);
        shader->Bind();
        glm::mat4 mvp = *projection * *view;
        glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3f(COLOR, .1, .1, .1);
        glDrawArrays(GL_LINES, 0, c * 8);
    }
    ~grid()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

class line {
public:
    Hd::Shader shader;
    glm::vec3 color;
    GLint MVP, POINTS, COLOR;
    glm::mat4 *view, *projection;
    glm::mat4 mvp;
    line(glm::mat4* v, glm::mat4* p)
        : shader("../res/shaders/line.glsl", "../res/shaders/frag.glsl")
        , view(v)
        , projection(p)
        , color(0.5f, 0.7f, 0.1f)

    {
        MVP = glGetUniformLocation(shader.Id(), "MVP");
        POINTS = glGetUniformLocation(shader.Id(), "POINTS");
        COLOR = glGetUniformLocation(shader.Id(), "COLOR");
    }
    ~line() { }
    void draw(glm::vec2 from, glm::vec2 to)
    {
        glm::vec2 a[2];
        a[0] = from;
        a[1] = to;
        shader.Bind();
        mvp = *projection * *view;
        glUniformMatrix4fv(MVP, 1, 0, glm::value_ptr(mvp));
        glUniform2fv(POINTS, 2, glm::value_ptr(a[0]));
        glUniform3fv(COLOR, 1, glm::value_ptr(color));
        glDrawArrays(GL_LINES, 0, 2);
    }

    void draw(glm::vec2* v, int count)
    {
        shader.Bind();
        mvp = *projection * *view;
        glUniformMatrix4fv(MVP, 1, 0, glm::value_ptr(mvp));
        glUniform3fv(COLOR, 1, glm::value_ptr(color));
        for (int i = 0; i < count - 1; i++) {
            glUniform2fv(POINTS, 2, glm::value_ptr(v[i]));
            glDrawArrays(GL_LINES, 0, 2);
        }
    }
};

class orbit : public line {
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods);

glm::vec2 get_viewportcoord();

float camera_zoom = 1.0 / 6371;
glm::vec2 camera_loc(0);

glm::mat4 view(1);
glm::mat4 projection(1);

glm::vec2 viewport_size(0);
GLFWwindow* w = 0;

bool drag = false;
glm::vec2 drag_start(0);

const int path_size = 50000;
const float delta_t = 500;
const float G = 6.67408e-17;
const float Me = 5.972e24;
const float Mm = 7.348e22;
const float moon_w = 2.42407e-6;

glm::vec2 path[2][path_size];
glm::vec2 init_loc(-6500, 0);
glm::vec2 init_vel(0, 260);
glm::vec2 moon_last(384400, 0);
float path_draw_time;

void calculate_path()
{
    path[0][0] = init_loc;
    path[1][0] = init_vel;

    for (int i = 1; i < path_size; i++) {
        glm::vec2 a(0);
        float l1 = glm::length(path[0][i - 1]);
        float l2 = glm::length(moon_last - path[0][i - 1]);
        a += G * Me / (l1 * l1) * glm::normalize(-path[0][i - 1]);
        a += G * Mm / (l2 * l2) * glm::normalize(moon_last - path[0][i - 1]);
        auto moon_angle = moon_w * delta_t;
        auto r = glm::mat2(cos(moon_angle), -sin(moon_angle), sin(moon_angle),
            cos(moon_angle));
        moon_last = r * moon_last;

        path[1][i] = path[1][i - 1] + a * delta_t / 1000.f;
        path[0][i] = path[0][i - 1] + path[1][i - 1] * delta_t / 1000.f;
    }
}

int main()
{

    view = glm::scale(view, glm::vec3(camera_zoom));
    view = glm::translate(view, glm::vec3(camera_loc, 0.0f));

    Hd::Window window("orbit", 1000, 1000);
    w = window.Id();
    Hd::Gui gui(window);

    glfwSetFramebufferSizeCallback(w, framebuffer_size_callback);
    glfwSetScrollCallback(w, scroll_callback);
    glfwSetMouseButtonCallback(w, mouse_button_callback);

    Hd::Shader circle_shader(
        "../res/shaders/vertex.glsl", "../res/shaders/frag.glsl");

    circle::init_gldata(&circle_shader, &view, &projection);

    circle earth(6371, glm::vec2(0, 0));
    circle moon(1737, glm::vec2(384400, 0));

    line l(&view, &projection);

    calculate_path();

    while (!window.ShouldClose()) {
        window.PollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        earth.draw();
        moon.draw();
        gui.Draw();

        path_draw_time = glfwGetTime();
        /* for (int i = 1; i < path_size; i++) { */
        /*     l.draw(path[0][i], path[0][i - 1]); */
        /* } */

        l.draw(&path[0][0], path_size);
        path_draw_time = glfwGetTime() - path_draw_time;

        if (drag) {
            auto a = get_viewportcoord() - drag_start;
            a.y /= viewport_size.x / viewport_size.y;
            a /= camera_zoom;
            view = glm::scale(glm::mat4(1), glm::vec3(camera_zoom));
            view = glm::translate(view, glm::vec3(a - camera_loc, 0.0));
        }

        window.SwapBuffers();
    }

    circle::destroy_gldata();
    return 0;
}

void Hd::Gui::GuiFunc()
{
    ImGui::Begin("Hello, world!");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    auto a = get_viewportcoord();
    a.y /= viewport_size.x / viewport_size.y;
    a /= camera_zoom;
    ImGui::Text("distance: %f, %f", (camera_loc + a).x, (camera_loc + a).y);

    ImGui::DragFloat2("init_vel", glm::value_ptr(init_vel));
    ImGui::Text("path_draw_time = %f", path_draw_time);

    if (ImGui::Button("calculate_path")) {
        calculate_path();
    }

    for (int i = 0; i < 5; i++) {
        ImGui::Text("path[%d] = (%.2f,%.2f)", i, path[0][i].x, path[0][i].y);
    }

    ImGui::End();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    viewport_size.x = width;
    viewport_size.y = height;

    projection
        = glm::scale(glm::mat4(1), glm::vec3(1, (float)width / height, 1));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0)
        camera_zoom *= .75;
    else
        camera_zoom /= .75;

    view = glm::scale(glm::mat4(1), glm::vec3(camera_zoom));
    view = glm::translate(view, glm::vec3(-camera_loc, 0.0f));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        drag = true;
        drag_start = get_viewportcoord();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        drag = false;

        auto a = get_viewportcoord() - drag_start;
        a.y /= viewport_size.x / viewport_size.y;
        a /= camera_zoom;
        camera_loc -= a;
    }
}

glm::vec2 get_viewportcoord()
{
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    glm::vec2 screencoord(x, y);

    auto r = screencoord / viewport_size;
    r.y = -r.y;
    return glm::vec2(2) * (r - glm::vec2(0.5, -0.5));
}
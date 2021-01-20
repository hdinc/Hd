// TODO: elasticity, 3d, compute shader, select and drag balls, friction and
// heat http://phet.colorado.edu/sims/collision-lab/collision-lab_en.html

// border scale bug?
// make count changeable
// ballbounce 1 frame lag

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Hd.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "balls.h"

void scroll_callback(double dx, double dy);
void framebuffer_size_callback(int x, int y);
void guifunc();

float radius = 0.4f;
float timescale = 1.0f;
float mtime = 0.0f;
float timebefore;
float deltatime;
bool pause = true;
float total_energy;
balls* gballs;
glm::vec2 border(10, 10);
Hd::Camera2D* gcam;

int main()
{
    Hd::Window::setSize(1280, 720);
    Hd::Window::setName("balls");
    auto& window = Hd::Window::getInstance();

    window.fpsLimit(60);
    window.ScrollCb.addFunc(scroll_callback);
    window.FrameBufferSizeCb.addFunc(framebuffer_size_callback);
    Hd::Camera2D cam;
    gcam = &cam;
    cam.zoom(0.1);
    Hd::Gui gui(window);
    gui.addFunc(guifunc);
    Hd::Shader borderShader(
        "../res/shaders/vertex.glsl", "../res/shaders/frag.glsl");

    GLint mvp = glGetUniformLocation(borderShader.Id(), "MVP");
    GLint color = glGetUniformLocation(borderShader.Id(), "COLOR");

    glClearColor(0, 0, 0, 1);

    GLuint vao_square, vbo_square;
    {
        glGenVertexArrays(1, &vao_square);
        glBindVertexArray(vao_square);
        glGenBuffers(1, &vbo_square);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square);
        float data[8]
            = { -border.x / 4, -border.y / 4, -border.x / 4, border.y / 4,
                  border.x / 4, border.y / 4, border.x / 4, -border.y / 4 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    timebefore = glfwGetTime();

    balls lballs(*cam.getVP(), radius, border);
    gballs = &lballs;

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);
        if (!pause) {
            deltatime = (timescale * (glfwGetTime() - timebefore));
            lballs.step(deltatime);
            total_energy = lballs.calculateTotalEnergy();
            mtime += deltatime;
        }
        timebefore = glfwGetTime();

        window.PollEvents();

        // draw border
        {
            borderShader.Bind();
            glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(*cam.getVP()));
            glBindVertexArray(vao_square);
            glUniform3f(color, 0, 1, 1);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }

        lballs.draw();
        gui.Draw();

        window.SwapBuffers();
    }
}

void guifunc()
{
    ImGui::Begin("properties");
    ImGui::Text("Total kinethic energy = %f", total_energy);

    ImGui::SliderFloat("timescale", &timescale, 0, 10);

    if (ImGui::SliderFloat("ball radius", &radius, .05f, 1.f)) {
        gballs->setRadius(radius);
    }

    ImGui::Text("Time = %.3f (%s)", mtime, pause ? "Paused" : "Playing");
    ImGui::Text("Deltatime = %.3f", deltatime);
    if (ImGui::Button("Pause")) {
        pause = !pause;
    }

    if (ImGui::Button("Step") && pause) {
        deltatime = (timescale * (0.01));
        gballs->step(deltatime);
        mtime += deltatime;
    }

    ImGui::Text("fps = %d", Hd::Window::getInstance().getFps());

    ImGui::End();
}

void scroll_callback(double dx, double dy)
{
    if (ImGui::IsAnyWindowHovered())
        return;
    Hd::Camera2D::onScroll(gcam, dx, dy);
}

void framebuffer_size_callback(int x, int y)
{
    Hd::Camera2D::onFramebufferSizeChange(gcam, x, y);
}

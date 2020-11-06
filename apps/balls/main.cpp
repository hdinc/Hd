// TODO: elasticity, 3d, compute shader, select and drag balls, friction and
// heat http://phet.colorado.edu/sims/collision-lab/collision-lab_en.html

#include <glad/glad.h>
#include <Hd.h>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include "ball.h"
#include <cstdio>

const float borderw = 20;
const float borderh = 12;

void initializeImGui(Hd::Window& window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window.Id(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void destroyImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

glm::vec2 rotate90(glm::vec2 x)
{
    glm::vec2 r;

    r.x = -x.y;
    r.y = x.x;

    return r;
}

void borderbounce(ball balls[], unsigned count)
{
    for (unsigned i = 0; i < count; i++) {
        if ((balls[i].loc.x > borderw / 2 - ball::radius
                && balls[i].speed.x > 0)
            || (balls[i].loc.x < -(borderw / 2 - ball::radius)
                && balls[i].speed.x < 0)) {
            balls[i].speed.x = -balls[i].speed.x;
        }
        if ((balls[i].loc.y > borderh / 2 - ball::radius
                && balls[i].speed.y > 0)
            || (balls[i].loc.y < -(borderh / 2 - ball::radius)
                && balls[i].speed.y < 0)) {
            balls[i].speed.y = -balls[i].speed.y;
        }
    }
}

void ballbounce(ball balls[], unsigned count)
{
    glm::vec2 ballAx, ballAy, ballBx, ballBy, AB, nAB;
    for (unsigned i = 0; i < count - 1; i++) {
        for (unsigned j = i + 1; j < count; j++) {
            if (glm::length(balls[i].loc - balls[j].loc) < 2 * ball::radius) {

                AB = glm::normalize(balls[j].loc - balls[i].loc);
                ballAx = AB * glm::dot(balls[i].speed, AB);
                ballBx = AB * glm::dot(balls[j].speed, AB);
                nAB = rotate90(AB);
                ballAy = nAB * glm::dot(balls[i].speed, nAB);
                ballBy = nAB * glm::dot(balls[j].speed, nAB);

                if (glm::dot(ballAx, ballBx) > 0) {
                    if (glm::dot(AB, ballAx) > 0) {
                        if (glm::length(ballAx) > glm::length(ballBx)) {
                            balls[i].speed = ballBx + ballAy;
                            balls[j].speed = ballAx + ballBy;
                        }
                    } else {
                        if (glm::length(ballBx) > glm::length(ballAx)) {
                            balls[i].speed = ballBx + ballAy;
                            balls[j].speed = ballAx + ballBy;
                        }
                    }
                } else if (glm::dot(ballAx, AB) > 0) {
                    balls[i].speed = ballBx + ballAy;
                    balls[j].speed = ballAx + ballBy;
                }
            }
        }
    }
}

void updateBalls(ball balls[], unsigned count, float deltatime)
{
    for (unsigned i = 0; i < count; i++) {
        balls[i].loc += balls[i].speed * deltatime;
    }
}

void drawBalls(ball balls[], unsigned count)
{

    for (unsigned i = 0; i < count - 1; i++) {
        ball::c = glm::vec4(.4, .4, .4, 1.f);
        balls[i].draw();
    }
    ball::c = glm::vec4(1, .2, .2, 1.f);
    balls[count - 1].draw();
}

int main()
{
    Hd::Window window("balls", 1280, 720);
    window.VSync(false);
    Hd::Shader shader("ball.vert", "ball.frag");
    initializeImGui(window);

    GLint mvp = glGetUniformLocation(shader.Id(), "MVP");
    GLint color = glGetUniformLocation(shader.Id(), "color");

    shader.Bind();

    glClearColor(0, 0, 0, 1);

    GLuint vao_square, vbo_square;
    {
        glGenVertexArrays(1, &vao_square);
        glBindVertexArray(vao_square);
        glGenBuffers(1, &vbo_square);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square);
        float data[8] = { -10, -6, -10, 6, 10, 6, 10, -6 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glm::mat4 projection(1);
    projection
        = glm::scale(glm::mat4(1), glm::vec3((9.0 / 16) / 7, 1.0 / 7, 1));

    ball::shader = &shader;
    ball::projection = &projection;
    float radius = .1f;
    ball::radius = radius;
    ball::initgldata();
    const int ballcount = 800;
    ball balls[ballcount];

    // initballs
    {
        for (unsigned i = 0; i < ballcount; i++) {
            balls[i].loc
                = glm::vec2(-9.85f + (i % 35) * .52f, -5.75f + (i / 35) * 0.5f);
            balls[i].speed = glm::vec2(
                glm::linearRand(-.001f, .001f), glm::linearRand(-.001f, .001f));
        }
        balls[ballcount - 1].speed = glm::vec2(10, 5);
    }

    float timescale = 1.0f;
    float time = 0.0f;
    float timebefore = glfwGetTime();
    float deltatime = 0;
    bool pause = true;
    float total_energy;

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        {
            glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(vao_square);
            glUniform4f(color, 1, 1, 1, 1);
            glDrawArrays(GL_LINE_LOOP, 0, 4);

            drawBalls(balls, ballcount);
        }

        // Calculate
        {

            if (!pause)
                time += deltatime = (timescale * (glfwGetTime() - timebefore));
            window.PollEvents();
            timebefore = glfwGetTime();

            if (!pause) {
                updateBalls(balls, ballcount, deltatime);
                borderbounce(balls, ballcount);
                ballbounce(balls, ballcount);
            }
            total_energy = 0;
            for (unsigned i = 0; i < ballcount; i++) {
                total_energy += (glm::length(balls[i].speed))
                    * (glm::length(balls[i].speed));
            }

            ball::radius = radius;
        }

        // Gui
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Hello, world!");

            {

                ImGui::Text("Total kinethic energy = %f", total_energy);
                ImGui::SliderFloat("timescale", &timescale, 0, 10);
                ImGui::SliderFloat("ball radius", &radius, .05f, 1.f);
                ImGui::Text("Time = %.3f", time);
                ImGui::Text("Deltatime = %.3f", deltatime);
                if (ImGui::Button("Pause")) {
                    pause = !pause;
                }
                if (ImGui::Button("DrawSpeeds")) {
                    ball::drawSpeed = !ball::drawSpeed;
                }
            }

            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        window.SwapBuffers();
    }

    ball::destroygldata();
    destroyImGui();
}

void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{

    // sdfsa
}

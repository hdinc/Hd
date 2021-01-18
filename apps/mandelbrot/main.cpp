#include <Hd.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void Gui();
void initializeImGui(GLFWwindow* window);
void destroyImGui();

static glm::mat4 mvp(1.0f);
static glm::vec2 loc(0.f);
static float zoom = 1;

int main()
{
    Hd::Window window("mandelbrot", 1280, 720);
    initializeImGui(window.Id());

    float square[] = { -1.f, -1.f, -1.f, 1.f, 1.f, -1.f, 1.f, 1.f };

    GLuint vao;
    GLuint vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    Hd::Shader shader(
        "../res/shaders/vertex.glsl", "../res/shaders/mandelbrot.glsl");
    shader.Bind();

    GLint u_mvp = glGetUniformLocation(shader.Id(), "MVP");

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        mvp = glm::translate(glm::mat4(1), glm::vec3(loc, 0.0f));
        mvp = glm::scale(mvp, glm::vec3(zoom));

        glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

        // gui
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            Gui();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        window.SwapBuffers();
        window.PollEvents();
    }

    destroyImGui();
}

void Gui()
{

    ImGui::Begin("Hello, world!");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat("zoom", &zoom);
    ImGui::SliderFloat2("loc", glm::value_ptr(loc), -1, 1);

    ImGui::End();
}

void initializeImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void destroyImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

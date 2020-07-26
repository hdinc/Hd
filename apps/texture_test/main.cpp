#include <Hd.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void Gui();
void initializeImGui(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void destroyImGui();

static glm::mat4 mvp(1.0f);
static glm::vec2 loc(0.f);
static float zoom = 1;
static float izoom = 1;

int main()
{
    Hd::Window window("test", 500, 500);
    Hd::Shader shader(
        "../res/shaders/vertex.glsl", "../res/shaders/texture.glsl");
    Hd::Texture texture("../res/textures/diyojen.png");

    glfwSetScrollCallback(window.Id(), scroll_callback);

    initializeImGui(window.Id());

    float square[] = { -1, -1, 0, 0, -1, 1, 0, 1, 1, -1, 1, 0, 1, 1, 1, 1 };

    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    texture.Bind(0);

    glClearColor(.0f, .0f, .0f, .0f);
    shader.Bind();

    glUniform1i(glGetUniformLocation(shader.Id(), "tex"), 0);
    GLint u_mvp = glGetUniformLocation(shader.Id(), "MVP");

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        if ((izoom - zoom > 0.01f) || (izoom - zoom < 0.01f))
            zoom += (izoom - zoom) / 8.f;

        mvp = glm::scale(glm::mat4(1), glm::vec3(zoom));
        mvp = glm::translate(mvp, glm::vec3(loc, 0.0f));
        glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    izoom += yoffset * izoom / 2;
}

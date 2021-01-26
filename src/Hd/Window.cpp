#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include <imgui.h>

#include <Hd/Window.h>
#include <Hd/WindowCallbacks.h>

static void glfw_error_callback(int error, const char* description);
static void GLAPIENTRY messagecallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam);

namespace Hd {

Window* gWindow;

const char* Hd::Window::mName = "Window";
int Hd::Window::mSize[2] = { 1000, 1000 };

Window& Window::getInstance()
{
    static Window window;
    return window;
}

Window::Window()
    : mFramebufferSize(mSize[0], mSize[1])
{

#ifndef NDEBUG
    glfwSetErrorCallback(glfw_error_callback);
#endif
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    puts("OPENGL DEBUG CONTEXT = ON");
#endif

    mWindowId = glfwCreateWindow(mSize[0], mSize[1], mName, NULL, NULL);

    glfwSetCursorPosCallback(mWindowId, cursor_position_callback);
    glfwSetKeyCallback(mWindowId, key_callback);
    glfwSetMouseButtonCallback(mWindowId, mouse_button_callback);
    glfwSetScrollCallback(mWindowId, scroll_callback);
    glfwSetFramebufferSizeCallback(mWindowId, framebuffer_size_callback);

    glfwMakeContextCurrent(mWindowId);
    gladLoadGL();

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(messagecallback, nullptr);
    glDebugMessageControl(
        GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif

    glfwSwapInterval(mVsync ? 1 : 0);

    gWindow = this;
}

Window::~Window()
{
    glfwDestroyWindow(mWindowId);
    glfwTerminate();
}

void Window::FullScreen(bool f)
{
    static bool fullscreen = false;
    static int posx, posy, wx, wy;
    if (f) {
        if (fullscreen) {
            return;
        }
        glfwGetWindowSize(mWindowId, &wx, &wy);
        glfwGetWindowPos(mWindowId, &posx, &posy);
        glfwSetWindowMonitor(mWindowId, glfwGetPrimaryMonitor(), 0, 0, 1920,
            1080, GLFW_DONT_CARE);
        glfwSwapInterval(mVsync ? 1 : 0);
        fullscreen = true;
        glViewport(0, 0, 1920, 1080);
        return;
    } else {
        if (fullscreen) {
            glfwSetWindowMonitor(
                mWindowId, NULL, posx, posy, wx, wy, GLFW_DONT_CARE);
            glfwSwapInterval(mVsync ? 1 : 0);
            fullscreen = false;
            glViewport(0, 0, wx, wy);
            return;
        }
        return;
    }
}

void Window::VSync(bool b)
{
    mVsync = b;
    glfwSwapInterval(mVsync ? 1 : 0);
}

void Window::waitFpsLimit()
{
    static double oldtime = 0;
    double t;
    t = glfwGetTime() - oldtime - 1.0 / (double)mFpsLimit;
    if (t < 0)
        std::this_thread::sleep_for(std::chrono::microseconds((int)(-t * 1e6)));
    oldtime = glfwGetTime();
}

void Window::fpsLimit(int a)
{
    if (a >= 10)
        mFpsLimit = a;
    else
        printf("[WINDOW]:cant limit fps under 10\n");
}

int Window::getFps()
{
    // TODO: fix me
    static int fps = 0;
    static double ltime = glfwGetTime();

    if (glfwGetTime() - ltime > 1.0) {
        ltime = glfwGetTime();
        fps = mFrame;
        mFrame = 0;
    }

    return fps;
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;

    glm::vec2 screencoord(xpos, ypos);

    auto r = screencoord / gWindow->mFramebufferSize;
    r.y = -r.y;
    gWindow->mMousePos = glm::vec2(2) * (r - glm::vec2(0.5, -0.5));

    for (auto f : gWindow->CursorPosCb.mFunctions)
        f.second(xpos, ypos);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;

    if (gWindow->hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

    for (auto f : gWindow->KeyCb.mFunctions)
        f.second(key, scancode, action, mods);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;

    if (gWindow->hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

    for (auto f : gWindow->MouseButtonCb.mFunctions)
        f.second(button, action, mods);
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;

    if (gWindow->hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

    for (auto f : gWindow->ScrollCb.mFunctions)
        f.second(xoffset, yoffset);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;

    gWindow->mFramebufferSize = glm::vec2(width, height);
    glViewport(0, 0, width, height);

    for (auto f : gWindow->FramebufferSizeCb.mFunctions)
        f.second(width, height);
}

}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void GLAPIENTRY messagecallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam)
{
    (void)userParam;
    (void)length;

    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    puts("---------------");
    printf("Debug message (%d): %s", id, message);

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        puts("Source: API");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        puts("Source: Window System");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        puts("Source: Shader Compiler");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        puts("Source: Third Party");
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        puts("Source: Application");
        break;
    case GL_DEBUG_SOURCE_OTHER:
        puts("Source: Other");
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        puts("Type: Error");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        puts("Type: Deprecated Behaviour");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        puts("Type: Undefined Behaviour");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        puts("Type: Portability");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        puts("Type: Performance");
        break;
    case GL_DEBUG_TYPE_MARKER:
        puts("Type: Marker");
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        puts("Type: Push Group");
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        puts("Type: Pop Group");
        break;
    case GL_DEBUG_TYPE_OTHER:
        puts("Type: Other");
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        puts("Severity: high");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        puts("Severity: medium");
        break;
    case GL_DEBUG_SEVERITY_LOW:
        puts("Severity: low");
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        puts("Severity: notification");
        break;
    }
    puts("");
}

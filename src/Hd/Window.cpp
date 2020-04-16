#include <stdio.h>
#include <glad/glad.h>

#include <Hd/Window.h>

static void framecallback(GLFWwindow* w, int width, int height);
static void glfw_error_callback(int error, const char* description);
static void GLAPIENTRY messagecallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

namespace Hd {

Window::Window(const char* name, int width, int height)
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

    m_WindowId = glfwCreateWindow(width, height, name, NULL, NULL);

    glfwSetFramebufferSizeCallback(m_WindowId, framecallback);
    glfwMakeContextCurrent(m_WindowId);
    gladLoadGL();

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(messagecallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif

    glfwSwapInterval(m_Vsync ? 1 : 0);
}

Window::~Window()
{
    glfwDestroyWindow(m_WindowId);
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
        glfwGetWindowSize(m_WindowId, &wx, &wy);
        glfwGetWindowPos(m_WindowId, &posx, &posy);
        glfwSetWindowMonitor(m_WindowId, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, GLFW_DONT_CARE);
        glfwSwapInterval(m_Vsync ? 1 : 0);
        fullscreen = true;
        glViewport(0, 0, 1920, 1080);
        return;
    } else {
        if (fullscreen) {
            glfwSetWindowMonitor(m_WindowId, NULL, posx, posy, wx, wy, GLFW_DONT_CARE);
            glfwSwapInterval(m_Vsync ? 1 : 0);
            fullscreen = false;
            glViewport(0, 0, wx, wy);
            return;
        }
        return;
    }
}

void Window::VSync(bool b)
{
    m_Vsync = b;
    glfwSwapInterval(m_Vsync ? 1 : 0);
}

}

static void framecallback(GLFWwindow* w, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void GLAPIENTRY messagecallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
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

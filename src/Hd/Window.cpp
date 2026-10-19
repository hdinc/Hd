#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <algorithm>

#include <Hd/Window.h>
#include <Hd/FunctionList.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif
#endif

static void glfw_error_callback(int error, const char* description);
static void GLAPIENTRY messagecallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message,
    const void* userParam);

namespace Hd {

Window* gWindow;

const char* Window::mName = "Window";
int Window::mSize[2] = { 1000, 1000 };
Window::glProfile Window::mGlProfile = Window::glProfile::core;

Window& Window::getInstance()
{
    static Window window;
    return window;
}

Window::Window()
{

#ifndef NDEBUG
    glfwSetErrorCallback(glfw_error_callback);
#endif
    if (!glfwInit())
        throw std::runtime_error("[WINDOW]: failed to initialize glfw");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    if (mGlProfile == glProfile::core)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    else
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    puts("OPENGL DEBUG CONTEXT = ON");
#endif

    mWindowId = glfwCreateWindow(mSize[0], mSize[1], mName, NULL, NULL);
    if (!mWindowId) {
        glfwTerminate();
        throw std::runtime_error("[WINDOW]: failed to create window / OpenGL 4.5 context");
    }

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

    framebuffer_size_callback(gWindow->Id(), mSize[0], mSize[1]);
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
    mVsyncFallback = 0;
    mVsyncFrames = 0;
}

// some drivers accept a swap interval but never wait, software rendering on
// a vnc or headless x server has no vblank. the first frames after vsync is
// enabled are timed and if they come much faster than the monitor refreshes,
// the frame rate is limited to the refresh rate instead.
void Window::checkVsync()
{
    if (!mVsync || mFpsLimit > 0 || mVsyncFallback || mVsyncFrames < 0)
        return;

    // skip the first frames, they include startup work
    const int skip = 10, measure = 60;
    if (mVsyncFrames++ == skip)
        mVsyncStart = glfwGetTime();
    if (mVsyncFrames < skip + measure)
        return;

    int refresh = 60;
    if (GLFWmonitor* m = glfwGetPrimaryMonitor())
        if (const GLFWvidmode* mode = glfwGetVideoMode(m))
            if (mode->refreshRate >= 30 && mode->refreshRate <= 360)
                refresh = mode->refreshRate;

    double interval = (glfwGetTime() - mVsyncStart) / measure;
    if (interval < 0.7 / refresh) {
        mVsyncFallback = refresh;
        printf("[WINDOW]:vsync doesn't wait, limiting fps to %d\n", refresh);
        fflush(stdout);
    }
    // measured once
    mVsyncFrames = -1;
}

// sleeps until the given glfw time. the os sleep stops about a millisecond
// early and the rest is spun, since wakeups are not precise enough to hit the
// deadline otherwise.
static void sleepUntil(double deadline)
{
#ifdef _WIN32
    // sleep_for uses the default ~15.6 ms timer resolution on windows, which is
    // useless for frame pacing. a high resolution waitable timer (windows 10
    // 1803+) wakes up within a fraction of a millisecond.
    static HANDLE timer = [] {
        HANDLE h = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!h)
            h = CreateWaitableTimerExW(nullptr, nullptr, 0, TIMER_ALL_ACCESS);
        return h;
    }();
#endif

    for (;;) {
        double left = deadline - glfwGetTime();
        if (left <= 0)
            return;
        if (left > 2e-3) {
#ifdef _WIN32
            // negative due time is relative, in 100 ns units
            LARGE_INTEGER due;
            due.QuadPart = -(LONGLONG)((left - 1e-3) * 1e7);
            if (timer && SetWaitableTimer(timer, &due, 0, nullptr, nullptr, FALSE))
                WaitForSingleObject(timer, INFINITE);
            else
                Sleep(1);
#else
            std::this_thread::sleep_for(std::chrono::duration<double>(left - 1e-3));
#endif
        } else {
            std::this_thread::yield();
        }
    }
}

void Window::waitFpsLimit()
{
    int limit = mFpsLimit > 0 ? mFpsLimit : mVsyncFallback;
    if (limit <= 0)
        return;

    // frames are scheduled against the previous deadline so the error of each
    // wait doesn't add up. a frame that is already late resets the schedule
    // instead of rushing the following ones.
    mNextFrame = std::max(mNextFrame + 1.0 / limit, glfwGetTime());
    sleepUntil(mNextFrame);
}

// limits the frame rate to a frames per second. 0 removes the limit.
void Window::fpsLimit(int a)
{
    if (a == 0 || a >= 10)
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

    gWindow->CursorPosCb.run(xpos, ypos);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;

    gWindow->KeyCb.run(key, scancode, action, mods);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;

    gWindow->MouseButtonCb.run(button, action, mods);
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;

    gWindow->ScrollCb.run(xoffset, yoffset);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;

    gWindow->mFramebufferSize = glm::vec2(width, height);
    gWindow->mFramebufferScale = (width > height)
        ? glm::vec2((float)height / width, 1)
        : glm::vec2(1, (float)width / height);

    glViewport(0, 0, width, height);

    gWindow->FramebufferSizeCb.run(width, height);
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

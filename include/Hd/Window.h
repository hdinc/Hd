#pragma once

#include <GLFW/glfw3.h>
#include <Hd/WindowCallbacks.h>
#include <glm/glm.hpp>

namespace Hd {

class Window {

public:
    static Window& getInstance();
    static void setName(const char* name);
    static void setSize(int x, int y);

    Window(Window const&) = delete;
    void operator=(Window const&) = delete;

    GLFWwindow* Id();
    void FullScreen(bool f);
    void VSync(bool v);
    void fpsLimit(int l);
    void SwapBuffers();
    void PollEvents();
    void WaitEvents();
    bool ShouldClose();
    int getFps();
    glm::vec2 getFramebufferSize();
    glm::vec2 getMousePos();

    // TODO: fix this smelly mess
    Callback<void (*)(void*, double, double), double, double>& CursorPosCb;
    Callback<void (*)(void*, double, double), double, double>& ScrollCb;
    Callback<void (*)(void*, int, int, int, int), int, int, int, int>& KeyCb;
    Callback<void (*)(void*, int, int, int), int, int, int>& MouseButtonCb;
    Callback<void (*)(void*, int, int), int, int>& FramebufferSizeCb;

private:
    Window(const char* name, int width, int height);
    ~Window();

    GLFWwindow* mWindowId;
    bool mVsync = false;
    int mFpsLimit = 60;
    int mFrame = 0;
    glm::vec2 mFramebufferSize;

    static const char* mName;
    static int mSize[2];

    void waitFpsLimit();

    //TODO: this is probably a bad practice
    friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

inline bool Window::ShouldClose() { return glfwWindowShouldClose(mWindowId); }

inline GLFWwindow* Window::Id() { return mWindowId; }

inline void Window::setName(const char* name) { mName = name; }

inline void Window::setSize(int x, int y)
{
    mSize[0] = x;
    mSize[1] = y;
}

inline void Window::SwapBuffers()
{
    glfwSwapBuffers(mWindowId);
    mFrame++;
}

inline void Window::PollEvents()
{
    waitFpsLimit();
    glfwPollEvents();
}

inline void Window::WaitEvents()
{
    waitFpsLimit();
    glfwWaitEvents();
}

inline glm::vec2 Window::getFramebufferSize()
{
    return mFramebufferSize;
}

}

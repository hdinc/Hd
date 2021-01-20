#pragma once

#include <GLFW/glfw3.h>
#include <Hd/WindowCallbacks.h>

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

    Callback<void (*)(double, double)>& CursorPosCb;
    Callback<void (*)(double, double)>& ScrollCb;
    Callback<void (*)(int, int, int, int)>& KeyCb;
    Callback<void (*)(int, int, int)>& MouseButtonCb;
    Callback<void (*)(int, int)>& FrameBufferSizeCb;

private:
    Window(const char* name, int width, int height);
    ~Window();

    GLFWwindow* mWindowId;
    bool mVsync = false;
    int mFpsLimit = 60;
    int mFrame = 0;

    static const char* mName;
    static int mSize[2];

    void waitFpsLimit();
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

}

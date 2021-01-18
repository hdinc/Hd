#pragma once

#include <GLFW/glfw3.h>
#include <Hd/WindowCallbacks.h>

namespace Hd {

class Window {

public:
    Window(const char* name, int width, int height);
    ~Window();
    GLFWwindow* Id() { return mWindowId; }
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
    GLFWwindow* mWindowId;
    bool mVsync = false;
    int mFpsLimit = 60;
    int mFrame = 0;

    void waitFpsLimit();
};

inline bool Window::ShouldClose() { return glfwWindowShouldClose(mWindowId); }

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

#pragma once

#include <GLFW/glfw3.h>

namespace Hd {

class Window {

public:
    Window(const char* name, int width, int height);
    ~Window();
    GLFWwindow* WindowId() { return m_WindowId; }
    void FullScreen(bool f);
    void VSync(bool v);
    void SwapBuffers();
    void PollEvents();
    void WaitEvents();
    bool ShouldClose();

private:
    GLFWwindow* m_WindowId;
    bool m_Vsync = true;
};

inline bool Window::ShouldClose()
{
    return glfwWindowShouldClose(m_WindowId);
}

inline void Window::SwapBuffers()
{
    glfwSwapBuffers(m_WindowId);
}

inline void Window::PollEvents()
{
    glfwPollEvents();
}

inline void Window::WaitEvents()
{
    glfwWaitEvents();
}

}

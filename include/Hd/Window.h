#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Hd/FunctionList.h>

#include <functional>

namespace Hd {

class Window {

public:
    enum class glProfile {
        core,
        compat
    };

public:
    static Window& getInstance();
    static void setName(const char* name);
    static void setSize(int x, int y);
    static void setGlProfile(glProfile profile);

    Window(Window const&) = delete;
    void operator=(Window const&) = delete;

    GLFWwindow* Id();
    void FullScreen(bool f);
    void VSync(bool v);
    void fpsLimit(int l);
    // calls frame until it returns false. in the browser the page calls it
    // once per animation frame and run returns at once: what frame uses
    // must stay alive.
    static void run(std::function<bool()> frame);

    void SwapBuffers();
    void PollEvents();
    void WaitEvents();
    // waits at most seconds for an event
    void WaitEventsTimeout(double seconds);
    bool ShouldClose();

    int getFps();
    glm::vec2 getFramebufferSize();
    // the monitor's scale for this window: 1.5 at 150% in Windows, 2 on a
    // Retina display. UI sizes are multiplied by it.
    float getContentScale() const { return mContentScale; }

    // where the window is, to put it back there next time: its normal
    // (not maximized) place in screen coordinates, and whether it is
    // maximized
    struct Geometry {
        int x = 0, y = 0, w = 0, h = 0;
        bool maximized = false;
    };
    Geometry getGeometry() const;
    // ignored when the place is off every monitor (one was unplugged)
    void setGeometry(const Geometry& g);
    // Windows' title bar in dark colours, to match a dark UI; elsewhere
    // nothing happens
    void setDarkTitleBar(bool dark);
    glm::vec2 getFramebufferScale();
    glm::vec2 getMousePos();
    void setMousePos(glm::vec2 p);

    FunctionList<void, double, double> CursorPosCb;
    FunctionList<void, double, double> ScrollCb;
    FunctionList<void, int, int, int, int> KeyCb;
    FunctionList<void, int, int, int> MouseButtonCb;
    FunctionList<void, int, int> FramebufferSizeCb;
    // the window moved to a monitor with another scale, or the scale changed
    FunctionList<void, float> ContentScaleCb;

private:
    Window();
    ~Window();

    GLFWwindow* mWindowId;
    bool mVsync = false;
    bool hasGui = false;
    int mFpsLimit = 0;
    double mNextFrame = 0;
    // vsync that doesn't block (no vblank, e.g. a vnc display) is replaced by
    // the fps limit at the refresh rate
    int mVsyncFallback = 0;
    int mVsyncFrames = 0;
    double mVsyncStart = 0;
    int mFrame = 0;
    glm::vec2 mFramebufferSize;
    glm::vec2 mFramebufferScale;
    glm::vec2 mMousePos;
    float mContentScale = 1;
    Geometry mNormal; // the last place while neither maximized nor minimized

    static const char* mName;
    static int mSize[2];
    static glProfile mGlProfile;

    void waitFpsLimit();
    void checkVsync();

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void content_scale_callback(GLFWwindow* window, float xscale, float yscale);
    static void window_pos_callback(GLFWwindow* window, int x, int y);
    static void window_size_callback(GLFWwindow* window, int w, int h);

    friend class Gui;
};

inline bool Window::ShouldClose() { return glfwWindowShouldClose(mWindowId); }

inline GLFWwindow* Window::Id() { return mWindowId; }

inline void Window::setName(const char* name) { mName = name; }

inline void Window::setSize(int x, int y)
{
    mSize[0] = x;
    mSize[1] = y;
}
inline void Window::setGlProfile(Window::glProfile profile) { mGlProfile = profile; }

inline void Window::SwapBuffers()
{
    glfwSwapBuffers(mWindowId);
    mFrame++;
    checkVsync();
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

inline void Window::WaitEventsTimeout(double seconds)
{
    waitFpsLimit();
    glfwWaitEventsTimeout(seconds);
}

inline glm::vec2 Window::getFramebufferSize()
{
    return mFramebufferSize;
}

inline glm::vec2 Window::getFramebufferScale()
{
    return mFramebufferScale;
}

// returns normalized coordinates of mouse position.
// (0,0) at center;
inline glm::vec2 Window::getMousePos()
{
    return mMousePos;
}

// sets the mouse position to normalized p.
inline void Window::setMousePos(glm::vec2 p)
{
    mMousePos = p;
    auto k = (glm::vec2(p.x, -p.y) + 1.0f) / 2.0f;
    glm::vec2 a = k * mFramebufferSize;
    glfwSetCursorPos(mWindowId, a.x, a.y);
}

}

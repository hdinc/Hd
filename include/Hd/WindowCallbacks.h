#pragma once

#include <GLFW/glfw3.h>

namespace Hd {

template <typename T>
class Callback {

public:
    int addFunc(T function);
    void removeFunc(int n);

private:
    static const int maxcount_ = 20;
    int mCount = 0;
    int mIdCount = 0;
    T mFunctions[maxcount_] = { 0 };
    int mIds[maxcount_] = { 0 };

    // this is probably a bad practice
    friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

}

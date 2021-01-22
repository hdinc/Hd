#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Hd/Window.h>
#include <imgui.h>

namespace Hd {

class Camera2D {

public:
    Camera2D();
    ~Camera2D();

    void lookAt(glm::vec2 l, float zoom);
    void move(glm::vec2 d);
    void zoom(float zoom);

    template <Camera2D& c, bool hasImgui>
    static void setupInput();

    glm::mat4* getView();
    glm::mat4* getProjection();
    glm::mat4* getVP();
    glm::vec2 getLoc();
    float getZoom();

private:
    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mVP;
    glm::vec2 mLoc;
    glm::vec2 mBorder;
    float mZoom;
    bool mDrag = false;
    glm::vec2 mDragStart;

    bool isinputSUp = false;
    int callbacks[4] = { 0 };

    void updateData();

    template <Camera2D& c, bool hasImgui>
    static void onScroll(double dx, double dy);

    template <Camera2D& c, bool hasImgui>
    static void onFramebufferSizeChange(int x, int y);

    template <Camera2D& c, bool hasImgui>
    static void onMouseMovement(double x, double y);

    template <Camera2D& c, bool hasImgui>
    static void onMouseButtonClick(int button, int action, int mods);
};

inline void Camera2D::zoom(float zoom)
{
    mZoom = zoom;
    updateData();
}

inline void Camera2D::move(glm::vec2 d)
{
    mLoc += d;
    updateData();
}

inline void Camera2D::lookAt(glm::vec2 l, float zoom)
{
    mLoc = l;
    mZoom = zoom;
    updateData();
}

inline void Camera2D::updateData()
{
    mView = glm::mat4(1);
    mView = glm::scale(mView, glm::vec3(mZoom, mZoom, 1));
    mView = glm::translate(mView, glm::vec3(-mLoc, 0));
    mVP = mProjection * mView;
}

inline glm::mat4* Camera2D::getView() { return &mView; }
inline glm::mat4* Camera2D::getProjection() { return &mProjection; }
inline glm::mat4* Camera2D::getVP() { return &mVP; }
inline glm::vec2 Camera2D::getLoc() { return mLoc; }
inline float Camera2D::getZoom() { return mZoom; }

template <Camera2D& c, bool hasImgui>
void Camera2D::onScroll(double dx, double dy)
{
    (void)dx;
    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }

    if (c.mDrag)
        return;

    float z = c.getZoom();
    z *= (dy > 0) ? 1.1 : 1 / 1.1;
    c.zoom(z);
}

template <Camera2D& c, bool hasImgui>
void Camera2D::onFramebufferSizeChange(int x, int y)
{
    auto s = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    c.mProjection = glm::scale(glm::mat4(1), glm::vec3(s, 1));
    c.mVP = c.mProjection * c.mView;
}

template <Camera2D& c, bool hasImgui>
void Camera2D::onMouseMovement(double x, double y)
{
    (void)x;
    (void)y;

    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }
    auto& w = Hd::Window::getInstance();
    if (c.mDrag) {
        auto a = w.getMousePos() - c.mDragStart;
        a.y /= w.getFramebufferSize().x / w.getFramebufferSize().y;
        a /= c.mZoom;
        c.mView = glm::scale(glm::mat4(1), glm::vec3(c.mZoom));
        c.mView = glm::translate(c.mView, glm::vec3(a - c.mLoc, 0.0));
    }
    c.mVP = c.mProjection * c.mView;
}

template <Camera2D& c, bool hasImgui>
void Camera2D::onMouseButtonClick(int button, int action, int mods)
{
    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }

    (void)mods;

    auto& w = Hd::Window::getInstance();

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && c.mDrag == false) {
        c.mDrag = true;
        c.mDragStart = w.getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && c.mDrag == true) {
        c.mDrag = false;

        auto a = w.getMousePos() - c.mDragStart;
        a.y /= w.getFramebufferSize().x / w.getFramebufferSize().y;
        a /= c.mZoom;
        c.mLoc -= a;
    }
    c.updateData();
}

template <Camera2D& c, bool hasImgui>
void Camera2D::setupInput()
{
    c.isinputSUp = true;
    auto& w = Hd::Window::getInstance();

    auto f1 = Camera2D::onScroll<c, hasImgui>;
    auto f2 = Camera2D::onFramebufferSizeChange<c, hasImgui>;
    auto f3 = Camera2D::onMouseMovement<c, hasImgui>;
    auto f4 = Camera2D::onMouseButtonClick<c, hasImgui>;

    c.callbacks[0] = w.ScrollCb.addFunc(f1);
    c.callbacks[1] = w.FramebufferSizeCb.addFunc(f2);
    c.callbacks[2] = w.CursorPosCb.addFunc(f3);
    c.callbacks[3] = w.MouseButtonCb.addFunc(f4);
}

}

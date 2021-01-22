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

    glm::mat4* getView();
    glm::mat4* getProjection();
    glm::mat4* getVP();
    glm::vec2 getLoc();
    float getZoom();

    template <bool hasImgui>
    void setupInput();

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

    template <bool hasImgui>
    static void onScroll(void* cam, double dx, double dy);

    template <bool hasImgui>
    static void onFramebufferSizeChange(void* cam, int x, int y);

    template <bool hasImgui>
    static void onMouseMovement(void* cam, double x, double y);

    template <bool hasImgui>
    static void onMouseButtonClick(void* cam, int button, int action, int mods);
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

template <bool hasImgui>
void Camera2D::onScroll(void* cam, double dx, double dy)
{
    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }

    (void)dx;
    auto c = static_cast<Camera2D*>(cam);

    if (c->mDrag)
        return;

    float z = c->getZoom();
    z *= (dy > 0) ? 1.1 : 1 / 1.1;
    c->zoom(z);
}

template <bool hasImgui>
void Camera2D::onFramebufferSizeChange(void* cam, int x, int y)
{
    auto c = static_cast<Camera2D*>(cam);
    auto s = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    c->mProjection = glm::scale(glm::mat4(1), glm::vec3(s, 1));
    c->mVP = c->mProjection * c->mView;
}

template <bool hasImgui>
void Camera2D::onMouseMovement(void* cam, double x, double y)
{
    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }

    (void)x;
    (void)y;

    auto c = static_cast<Camera2D*>(cam);
    auto& w = Hd::Window::getInstance();

    if (c->mDrag) {
        auto a = w.getMousePos() - c->mDragStart;
        a.y /= w.getFramebufferSize().x / w.getFramebufferSize().y;
        a /= c->mZoom;
        c->mView = glm::scale(glm::mat4(1), glm::vec3(c->mZoom));
        c->mView = glm::translate(c->mView, glm::vec3(a - c->mLoc, 0.0));
    }
    c->mVP = c->mProjection * c->mView;
}

template <bool hasImgui>
void Camera2D::onMouseButtonClick(void* cam, int button, int action, int mods)
{
    if (hasImgui) {
        if (ImGui::IsAnyWindowHovered())
            return;
    }

    (void)mods;

    auto c = static_cast<Camera2D*>(cam);

    auto& w = Hd::Window::getInstance();

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && c->mDrag == false) {
        c->mDrag = true;
        c->mDragStart = w.getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && c->mDrag == true) {
        c->mDrag = false;

        auto a = w.getMousePos() - c->mDragStart;
        a.y /= w.getFramebufferSize().x / w.getFramebufferSize().y;
        a /= c->mZoom;
        c->mLoc -= a;
    }
    c->updateData();
}

template <bool hasImgui>
void Camera2D::setupInput()
{
    isinputSUp = true;
    auto& w = Hd::Window::getInstance();

    auto f1 = Camera2D::onScroll<hasImgui>;
    auto f2 = Camera2D::onFramebufferSizeChange<hasImgui>;
    auto f3 = Camera2D::onMouseMovement<hasImgui>;
    auto f4 = Camera2D::onMouseButtonClick<hasImgui>;

    callbacks[0] = w.ScrollCb.addFunc(this, f1);
    callbacks[1] = w.FramebufferSizeCb.addFunc(this, f2);
    callbacks[2] = w.CursorPosCb.addFunc(this, f3);
    callbacks[3] = w.MouseButtonCb.addFunc(this, f4);
}

}

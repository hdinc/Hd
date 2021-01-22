#include <Hd/Camera2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

Camera2D::Camera2D()
    : mView(1)
    , mProjection(1)
    , mZoom(1)
{
}

Camera2D::~Camera2D()
{
    if (isinputSUp) {

        auto& w = Hd::Window::getInstance();
        w.ScrollCb.removeFunc(callbacks[0]);
        w.FramebufferSizeCb.removeFunc(callbacks[1]);
        w.CursorPosCb.removeFunc(callbacks[2]);
        w.MouseButtonCb.removeFunc(callbacks[3]);
    }
}

void Camera2D::onScroll(void* cam, double dx, double dy)
{
    if (Window::getInstance().hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

    (void)dx;
    auto c = static_cast<Camera2D*>(cam);

    if (c->mDrag)
        return;

    float z = c->getZoom();
    z *= (dy > 0) ? 1.1 : 1 / 1.1;
    c->zoom(z);
}

void Camera2D::onFramebufferSizeChange(void* cam, int x, int y)
{
    auto c = static_cast<Camera2D*>(cam);
    auto s = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    c->mProjection = glm::scale(glm::mat4(1), glm::vec3(s, 1));
    c->mVP = c->mProjection * c->mView;
}

void Camera2D::onMouseMovement(void* cam, double x, double y)
{
    if (Window::getInstance().hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

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

void Camera2D::onMouseButtonClick(void* cam, int button, int action, int mods)
{
    if (Window::getInstance().hasGui)
        if (ImGui::IsAnyWindowHovered())
            return;

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

void Camera2D::setupInput()
{
    isinputSUp = true;
    auto& w = Hd::Window::getInstance();

    auto f1 = Camera2D::onScroll;
    auto f2 = Camera2D::onFramebufferSizeChange;
    auto f3 = Camera2D::onMouseMovement;
    auto f4 = Camera2D::onMouseButtonClick;

    callbacks[0] = w.ScrollCb.addFunc(this, f1);
    callbacks[1] = w.FramebufferSizeCb.addFunc(this, f2);
    callbacks[2] = w.CursorPosCb.addFunc(this, f3);
    callbacks[3] = w.MouseButtonCb.addFunc(this, f4);
}

}

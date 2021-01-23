#include <Hd/Camera2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

extern Window* gWindow;

Camera2D::Camera2D()
    : mView(1)
    , mProjection(1)
    , mLoc(0)
    , mZoom(1)
{
}

Camera2D::~Camera2D()
{
    if (isinputSUp) {

        gWindow->ScrollCb.removeFunc(callbacks[0]);
        gWindow->FramebufferSizeCb.removeFunc(callbacks[1]);
        gWindow->CursorPosCb.removeFunc(callbacks[2]);
        gWindow->MouseButtonCb.removeFunc(callbacks[3]);
    }
}

void Camera2D::onScroll(void* cam, double dx, double dy)
{
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
    c->mFramebufferScale = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    c->mProjection = glm::scale(glm::mat4(1), glm::vec3(c->mFramebufferScale, 1));
    c->mVP = c->mProjection * c->mView;
}

void Camera2D::onMouseMovement(void* cam, double x, double y)
{
    (void)x;
    (void)y;

    auto c = static_cast<Camera2D*>(cam);

    if (c->mDrag) {
        auto a = gWindow->getMousePos() - c->mDragStart;

        a /= c->mFramebufferScale;
        a /= c->mZoom;
        c->mView = glm::scale(glm::mat4(1), glm::vec3(c->mZoom));
        c->mView = glm::translate(c->mView, glm::vec3(a - c->mLoc, 0.0));
    }
    c->mVP = c->mProjection * c->mView;
}

void Camera2D::onMouseButtonClick(void* cam, int button, int action, int mods)
{
    (void)mods;

    auto c = static_cast<Camera2D*>(cam);

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && c->mDrag == false) {
        c->mDrag = true;
        c->mDragStart = gWindow->getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && c->mDrag == true) {
        c->mDrag = false;

        auto a = gWindow->getMousePos() - c->mDragStart;
        a /= c->mFramebufferScale;
        a /= c->mZoom;
        c->mLoc -= a;
    }
    c->updateData();
}

void Camera2D::setupInput()
{
    isinputSUp = true;

    auto f1 = Camera2D::onScroll;
    auto f2 = Camera2D::onFramebufferSizeChange;
    auto f3 = Camera2D::onMouseMovement;
    auto f4 = Camera2D::onMouseButtonClick;

    callbacks[0] = gWindow->ScrollCb.addFunc(this, f1);
    callbacks[1] = gWindow->FramebufferSizeCb.addFunc(this, f2);
    callbacks[2] = gWindow->CursorPosCb.addFunc(this, f3);
    callbacks[3] = gWindow->MouseButtonCb.addFunc(this, f4);
}

}

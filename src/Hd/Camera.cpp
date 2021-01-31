#include <Hd/Camera.h>
#include <Hd/Window.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

namespace Hd {

extern Window* gWindow;

Camera::Camera()
    : mView(1)
    , mProjection(1)
    , mLoc(0)
    , mZoom(1)
{
    auto s = gWindow->getFramebufferSize();
    onFramebufferSizeChange(s.x, s.y);
}

Camera::~Camera()
{
    if (inputSUp) {
        gWindow->ScrollCb.remove(ids[0]);
        gWindow->FramebufferSizeCb.remove(ids[1]);
        gWindow->CursorPosCb.remove(ids[2]);
        gWindow->MouseButtonCb.remove(ids[3]);
    }
}

void Camera::onScroll(double dx, double dy)
{
    (void)dx;

    if (mDrag)
        return;

    mZoom *= (dy > 0) ? 1.1 : 1 / 1.1;
    updateData();
}

void Camera::onFramebufferSizeChange(int x, int y)
{
    (void)x;
    (void)y;

    mProjectionScale = gWindow->getFramebufferScale();
    mProjection = glm::scale(glm::mat4(1), glm::vec3(mProjectionScale, 1));
    mVP = mProjection * mView;
}

void Camera::onMouseMovement(double x, double y)
{
    (void)x;
    (void)y;

    if (mDrag) {
        auto a = gWindow->getMousePos() - mDragStart;

        a /= mProjectionScale;
        a /= mZoom;
        mView = glm::scale(glm::mat4(1), glm::vec3(mZoom));
        mView = glm::translate(mView, glm::vec3(a - mLoc, 0.0));
    }
    mVP = mProjection * mView;
}

void Camera::onMouseButtonClick(int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && mDrag == false) {
        mDrag = true;
        mDragStart = gWindow->getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && mDrag == true) {
        mDrag = false;

        auto a = gWindow->getMousePos() - mDragStart;
        a /= mProjectionScale;
        a /= mZoom;
        mLoc -= a;
        updateData();
    }
}

void Camera::setupInput()
{
    inputSUp = true;

    using namespace std::placeholders;
    auto f1 = std::bind(&Camera::onScroll, this, _1, _2);
    auto f2 = std::bind(&Camera::onFramebufferSizeChange, this, _1, _2);
    auto f3 = std::bind(&Camera::onMouseMovement, this, _1, _2);
    auto f4 = std::bind(&Camera::onMouseButtonClick, this, _1, _2, _3);

    ids[0] = gWindow->ScrollCb.add(f1);
    ids[1] = gWindow->FramebufferSizeCb.add(f2);
    ids[2] = gWindow->CursorPosCb.add(f3);
    ids[3] = gWindow->MouseButtonCb.add(f4);
}

}

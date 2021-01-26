#include <Hd/Camera2D.h>
#include <Hd/Window.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

namespace Hd {

extern Window* gWindow;

Camera2D::Camera2D()
    : mView(1)
    , mProjection(1)
    , mLoc(0)
    , mZoom(1)
{
    int x, y;
    glfwGetFramebufferSize(gWindow->Id(), &x, &y);
    onFramebufferSizeChange(x, y);
}

Camera2D::~Camera2D()
{
    if (inputSUp) {
        gWindow->ScrollCb.remove(ids[0]);
        gWindow->FramebufferSizeCb.remove(ids[1]);
        gWindow->CursorPosCb.remove(ids[2]);
        gWindow->MouseButtonCb.remove(ids[3]);
    }
}

void Camera2D::onScroll(double dx, double dy)
{
    (void)dx;

    if (mDrag)
        return;

    float z = getZoom();
    z *= (dy > 0) ? 1.1 : 1 / 1.1;
    zoom(z);
}

void Camera2D::onFramebufferSizeChange(int x, int y)
{
    mFramebufferScale = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    mProjection = glm::scale(glm::mat4(1), glm::vec3(mFramebufferScale, 1));
    mVP = mProjection * mView;
}

void Camera2D::onMouseMovement(double x, double y)
{
    (void)x;
    (void)y;

    if (mDrag) {
        auto a = gWindow->getMousePos() - mDragStart;

        a /= mFramebufferScale;
        a /= mZoom;
        mView = glm::scale(glm::mat4(1), glm::vec3(mZoom));
        mView = glm::translate(mView, glm::vec3(a - mLoc, 0.0));
    }
    mVP = mProjection * mView;
}

void Camera2D::onMouseButtonClick(int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && mDrag == false) {
        mDrag = true;
        mDragStart = gWindow->getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE && mDrag == true) {
        mDrag = false;

        auto a = gWindow->getMousePos() - mDragStart;
        a /= mFramebufferScale;
        a /= mZoom;
        mLoc -= a;
    }
    updateData();
}

void Camera2D::setupInput()
{
    inputSUp = true;

    using namespace std::placeholders;
    auto f1 = std::bind(&Camera2D::onScroll, this, _1, _2);
    auto f2 = std::bind(&Camera2D::onFramebufferSizeChange, this, _1, _2);
    auto f3 = std::bind(&Camera2D::onMouseMovement, this, _1, _2);
    auto f4 = std::bind(&Camera2D::onMouseButtonClick, this, _1, _2, _3);

    ids[0] = gWindow->ScrollCb.add(f1);
    ids[1] = gWindow->FramebufferSizeCb.add(f2);
    ids[2] = gWindow->CursorPosCb.add(f3);
    ids[3] = gWindow->MouseButtonCb.add(f4);
}

}

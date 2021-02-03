#include <Hd/Camera.h>
#include <Hd/Window.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

namespace Hd {

extern Window* gWindow;

Camera::Camera()
{
    auto s = gWindow->getFramebufferSize();
    onFramebufferSizeChange(s.x, s.y);

    using namespace std::placeholders;
    auto f = std::bind(&Camera::onFramebufferSizeChange, this, _1, _2);
    ids[0] = gWindow->FramebufferSizeCb.add(f);

    if (mProjectionType == projectionType::perspective) {
        setPerspective(mPerspectiveFOV);
    } else {
        setOrthographic();
    }
}

Camera::~Camera()
{
    if (inputEnabled) {
        disableInput();
    }
    gWindow->FramebufferSizeCb.remove(ids[0]);
}

void Camera::onScroll(double dx, double dy)
{
    (void)dx;

    if (mDrag)
        return;

    glm::vec3 v = mLoc - mTarget;

    move(-v * (float)dy / 10.0f);
}

void Camera::onFramebufferSizeChange(int x, int y)
{
    (void)x;
    (void)y;

    mProjectionScale = gWindow->getFramebufferScale();
    if (mProjectionType == projectionType::perspective) {
        setPerspective(mPerspectiveFOV);
    } else {
        setOrthographic();
    }
    calculateVP();
}

void Camera::onMouseMovement(double x, double y)
{
    (void)x;
    (void)y;

    if (mDrag) {
        auto a = gWindow->getMousePos() - mDragStart;

        // broken
        a /= mProjectionScale;
        glm::vec3 rotateAxis = glm::normalize(glm::cross(mTarget - mLoc, glm::vec3(a, 0.0f)));
        glm::vec3 nloc = glm::rotate(glm::mat4(1), glm::length(a), rotateAxis) * glm::vec4(mLoc, 1.0f);
        mView = glm::lookAt(nloc, mTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        mVP = mProjection * mView;
    }
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

        // broken
        auto a = gWindow->getMousePos() - mDragStart;
        a /= mProjectionScale;
        glm::vec3 rotateAxis = glm::normalize(glm::cross(mTarget - mLoc, glm::vec3(a, 0.0f)));
        mLoc = glm::rotate(glm::mat4(1), glm::length(a), rotateAxis) * glm::vec4(mLoc, 1.0f);
        calculateView();
        calculateVP();
    }
}

void Camera::enableInput()
{
    inputEnabled = true;

    using namespace std::placeholders;
    auto f1 = std::bind(&Camera::onScroll, this, _1, _2);
    auto f2 = std::bind(&Camera::onMouseMovement, this, _1, _2);
    auto f3 = std::bind(&Camera::onMouseButtonClick, this, _1, _2, _3);

    ids[1] = gWindow->ScrollCb.add(f1);
    ids[2] = gWindow->CursorPosCb.add(f2);
    ids[3] = gWindow->MouseButtonCb.add(f3);
}

void Camera::disableInput()
{
    inputEnabled = false;
    gWindow->ScrollCb.remove(ids[1]);
    gWindow->CursorPosCb.remove(ids[2]);
    gWindow->MouseButtonCb.remove(ids[3]);
}

}

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

    calculateView();

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
    mPolarLoc.x *= (dy < 0) ? 1.1 : 1 / 1.1;
    calculateView();
    calculateVP();
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

    auto pos = gWindow->getMousePos();
    auto a = pos - mDragStart;
    mDragStart = pos;

    a /= mProjectionScale;

    if (mDrag) {
        mPolarLoc.y -= 2 * a.y;
        mPolarLoc.z -= 2 * a.x;
        calculateView();
        calculateVP();
    }
}

void Camera::onMouseButtonClick(int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        mDrag = true;
        glfwSetInputMode(gWindow->Id(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mDragStart = gWindow->getMousePos();
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        mDrag = false;
        glfwSetInputMode(gWindow->Id(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

void Camera::setOrthographic()
{
    // TODO: fix me
    mProjectionType = projectionType::orthographic;

    /* glm::vec2 v = 1.0f / mProjectionScale; */
    /* auto loc = glm::vec3(; */

    /* auto d = glm::length(mLoc - mTarget); */
    /* mProjection = glm::ortho(-v.x * d, v.x * d, -v.y * d, v.y * d, -1000000.f, 1000000.f); */
    /* calculateVP(); */
}

void Camera::setPerspective(float fov)
{
    mProjectionType = projectionType::perspective;
    mPerspectiveFOV = fov;
    float aspect = mProjectionScale.y / mProjectionScale.x;
    mProjection = glm::perspective(fov, aspect, 0.01f, 100000.0f);
    calculateVP();
}

void Camera::calculateView()
{
    /* mView = glm::lookAt(toEuclidean(mPolarLoc), toEuclidean(mPolarTarget), mUp); */

    glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -mPolarLoc.x));
    glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, this->mPolarLoc.y, glm::vec3(1.f, 0.f, 0.f));
    mView = glm::rotate(ViewRotateX, -this->mPolarLoc.z, glm::vec3(0.f, 1.f, 0.f));
}

void Camera::calculateVP()
{
    mVP = mProjection * mView;
}

glm::vec3 Camera::toEuclidean(glm::vec3 v)
{
    // TODO: why?
    glm::vec3 r;
    r.x = v.x * cos(v.y) * sin(v.z);
    r.y = v.x * sin(v.y);
    r.z = v.x * cos(v.y) * cos(v.z);

    return r;
}

glm::vec3 Camera::toPolar(glm::vec3 v)
{
    // TODO: fix zero condition better way
    glm::vec3 r;
    r.x = glm::length(v);
    r.y = atan(glm::length(glm::vec2(v.x, v.z)) / v.y);
    r.z = v.z == 0 ? 0 : atan(v.x / v.z);

    return r;
}

}

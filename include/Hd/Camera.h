#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

class Camera {

public:
    enum class projectionType {
        perspective,
        orthographic
    };

public:
    Camera();
    ~Camera();

    void setPerspective(float fov);
    void setOrthographic();

    void setLoc(glm::vec3 loc);
    void move(glm::vec3 dloc);

    glm::mat4* getView();
    glm::mat4* getProjection();
    glm::mat4* getVP();
    glm::vec3 getLoc();

    void enableInput();
    void disableInput();

private:
    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mVP;
    glm::vec3 mLoc = glm::vec3(0, 1, 1);
    glm::vec3 mTarget = glm::vec3(0);
    glm::vec2 mProjectionScale;

    float mPerspectiveFOV = glm::radians(45.0f);

    projectionType mProjectionType = projectionType::perspective;

    bool mDrag = false;
    glm::vec2 mDragStart;

    void calculateView();
    void calculateVP();

    bool inputEnabled = false;
    int ids[4] = { 0 };

    void onScroll(double dx, double dy);
    void onFramebufferSizeChange(int x, int y);
    void onMouseMovement(double x, double y);
    void onMouseButtonClick(int button, int action, int mods);
};

inline void Camera::setOrthographic()
{
    mProjectionType = projectionType::orthographic;

    glm::vec2 v = 1.0f / mProjectionScale;
    mProjection = glm::ortho(-v.x, v.x, -v.y, v.y, -1000.f, 1000.f);
    calculateVP();
}

inline void Camera::setPerspective(float fov)
{
    mProjectionType = projectionType::perspective;
    mPerspectiveFOV = fov;
    float aspect = mProjectionScale.y / mProjectionScale.x;
    mProjection = glm::perspective(fov, aspect, 0.01f, 10000.0f);
    calculateVP();
}

inline void Camera::setLoc(glm::vec3 loc)
{
    mLoc = loc;
    calculateView();
    calculateVP();
}

inline void Camera::move(glm::vec3 dloc)
{
    mLoc += dloc;
    calculateView();
    calculateVP();
}

inline void Camera::calculateView()
{
    mView = glm::lookAt(mLoc, mTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

inline void Camera::calculateVP()
{
    mVP = mProjection * mView;
}

inline glm::mat4* Camera::getView() { return &mView; }
inline glm::mat4* Camera::getProjection() { return &mProjection; }
inline glm::mat4* Camera::getVP() { return &mVP; }
inline glm::vec3 Camera::getLoc() { return mLoc; }

}

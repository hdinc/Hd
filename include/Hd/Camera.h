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

    glm::mat4* getView() { return &mView; };
    glm::mat4* getProjection() { return &mProjection; };
    glm::mat4* getVP() { return &mVP; };

    void setLoc(glm::vec3 v)
    {
        mPolarLoc = toPolar(v);
        mLoc = v;
        calculateView();
        calculateVP();
    }

    glm::vec3 getLoc()
    {
        return (mProjectionType == projectionType::perspective) ? toEuclidean(mPolarLoc) : mLoc;
    }

    void enableInput();
    void disableInput();

private:
    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mVP;

    glm::vec3 mPolarLoc = glm::vec3(10, glm::pi<float>() / 4.0f, 0.0f);
    glm::vec3 mPolarTarget = glm::vec3(0);

    glm::vec3 mLoc = glm::vec3(0, 0, 10);
    glm::vec3 mTarget = glm::vec3(0, 0, 0);

    glm::vec2 mProjectionScale;

    float mPerspectiveFOV = glm::radians(45.0f);

    projectionType mProjectionType = projectionType::perspective;

    bool mDrag = false;
    glm::vec2 mDragStart;

    bool inputEnabled = false;
    int ids[4] = { 0 };

private:
    void calculateView();
    void calculateVP();

    void onScroll(double dx, double dy);
    void onFramebufferSizeChange(int x, int y);
    void onMouseMovement(double x, double y);
    void onMouseButtonClick(int button, int action, int mods);

    glm::vec3 toEuclidean(glm::vec3 v);
    glm::vec3 toPolar(glm::vec3 v);
};

}

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

    static void onScroll(void* cam, double dx, double dy);
    static void onFramebufferSizeChange(void* cam, int x, int y);
    static void onMouseMovement(void* cam, double x, double y);
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

}

#pragma once

#include <glm/glm.hpp>

namespace Hd {

class Camera2D {

public:
    Camera2D();
    ~Camera2D();
    void lookAt(float x, float y, float zoom);
    void move(float dx, float dy);
    void zoom(float zoom);
    void upload();
    glm::vec2 getLoc();
    float getZoom();
    glm::mat4* getView();
    glm::mat4* getProjection();
    glm::mat4 getVP();

private:
    glm::mat4 m_View;
    glm::mat4 m_Projection;
    float m_X, m_Y, m_Zoom;
};

inline glm::vec2 Camera2D::getLoc() { return glm::vec2(m_X, m_Y); }

inline float Camera2D::getZoom() { return m_Zoom; }

inline void Camera2D::zoom(float zoom) { m_Zoom = zoom; }

inline void Camera2D::move(float dx, float dy)
{
    m_X += dx;
    m_Y += dy;
}

inline void Camera2D::lookAt(float x, float y, float zoom)
{
    m_X = x;
    m_Y = y;
    m_Zoom = zoom;
}

inline glm::mat4* Camera2D::getView() { return &m_View; }

inline glm::mat4* Camera2D::getProjection() { return &m_Projection; }

}

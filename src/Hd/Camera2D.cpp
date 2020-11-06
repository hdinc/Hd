#include <Hd/Camera2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

Camera2D::Camera2D()
    : m_View(1)
    , m_Projection(1)
    , m_X(0)
    , m_Y(0)
    , m_Zoom(1)
{
}

Camera2D::~Camera2D() { }

glm::mat4 Camera2D::getVP()
{
    glm::mat4 vp(1);
    vp = glm::translate(vp, glm::vec3(m_X, m_Y, 0));
    vp = glm::scale(vp, glm::vec3(m_Zoom));
    return vp;
}

}

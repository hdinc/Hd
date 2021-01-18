#include <Hd/Camera2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

Camera2D::Camera2D()
    : mView(1)
    , mProjection(1)
    , mZoom(1)
{
}

Camera2D::~Camera2D()
{
}

void Camera2D::onScroll(Camera2D* c, double dx, double dy)
{
    (void)dx;
    float z = c->getZoom();
    z *= (dy > 0) ? 1.1 : 1 / 1.1;
    c->zoom(z);
}

void Camera2D::onFramebufferSizeChange(Camera2D* c, int x, int y)
{
    auto s = (x > y) ? glm::vec2((float)y / x, 1) : glm::vec2(1, (float)x / y);
    c->mProjection = glm::scale(glm::mat4(1), glm::vec3(s, 1));
    c->mVP = c->mProjection * c->mView;
}

}

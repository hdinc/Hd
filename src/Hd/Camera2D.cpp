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

}

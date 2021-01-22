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
    if (isinputSUp) {

        auto& w = Hd::Window::getInstance();
        w.ScrollCb.removeFunc(callbacks[0]);
        w.FramebufferSizeCb.removeFunc(callbacks[1]);
        w.CursorPosCb.removeFunc(callbacks[2]);
        w.MouseButtonCb.removeFunc(callbacks[3]);
    }
}

}

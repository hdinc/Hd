#include <Hd/CamCtrl.h>
#include <Hd/Window.h>

#include <functional>

namespace Hd {

CamCtrl::CamCtrl(Hd::Camera& c)
    : cam { c }
{

    moving = false;

    auto& window = Hd::Window::getInstance();

    using namespace std::placeholders;
    fscb = window.FramebufferSizeCb.add(std::bind(&CamCtrl::fsfun, this, _1, _2));
    mbutcb = window.MouseButtonCb.add(std::bind(&CamCtrl::mbutfun, this, _1, _2));
    mposcb = window.CursorPosCb.add(std::bind(&CamCtrl::mposfun, this, _1, _2));
}

CamCtrl::~CamCtrl()
{
    auto& window = Hd::Window::getInstance();
    window.FramebufferSizeCb.remove(fscb);
    window.MouseButtonCb.remove(mbutcb);
    window.CursorPosCb.remove(mposcb);
}

}

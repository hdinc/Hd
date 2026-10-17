#include <Hd/CamCtrl2D.h>
#include <Hd/Window.h>
#include <imgui.h>

namespace Hd {

CamCtrl2D::CamCtrl2D(Hd::Camera& c)
    : CamCtrl(c)
{
    // look down the -z axis with +y up, so the xy plane maps to the screen without mirroring
    cam.setDir(-glm::pi<float>() / 2.0f, 0);
    auto p = cam.getPos();
    if (p.z <= 0.0f)
        cam.setPos({ p.x, p.y, 10.0f });
    auto s = Hd::Window::getInstance().getFramebufferSize();
    fsfun(s.x, s.y);
}

void CamCtrl2D::fsfun(float x, float y)
{
    cam.setfbsize({ x, y });
    cam.setOrtho();
}

void CamCtrl2D::mbutfun(int but, bool press)
{
    if (but == GLFW_MOUSE_BUTTON_RIGHT) {
        if (!moving && press) {
            movloc = Hd::Window::getInstance().getMousePos();
            movdir = cam.getDir();
            movpos = cam.getPos();
        }

        if (moving && !press) {
        }

        moving = press;
    }
}

void CamCtrl2D::mposfun(float, float)
{
    if (moving) {

        auto& window = Hd::Window::getInstance();
        glm::vec2 diff = window.getMousePos() - movloc;
        glm::vec2 fb = window.getFramebufferSize();

        // mouse position is in ndc, convert to world units so the plane follows the cursor
        glm::vec2 halfsize { movpos.z * fb.x / fb.y, movpos.z };
        cam.setPos(movpos - glm::vec3(diff * halfsize, 0.0f));
    }
}

void CamCtrl2D::update() {}

void CamCtrl2D::gui()
{

    static glm::vec3 m { 0.0f };
    static glm::vec3 r { 0.0f };
    static glm::vec3 mc { 0.0f };

    if (ImGui::SliderFloat3("move", &m.x, -0.5f, 0.5f))
        cam.move(m);
    else
        m = glm::vec3(0.0f);

    if (ImGui::SliderFloat3("rotate", &r.x, -0.5f, 0.5f))
        cam.rotate(r);
    else
        r = glm::vec3(0.0f);

    if (ImGui::SliderFloat3("movc", &mc.x, -0.5f, 0.5f))
        cam.moveC(mc);
    else
        mc = glm::vec3(0.0f);
}

}

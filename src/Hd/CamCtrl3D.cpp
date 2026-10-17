#include <Hd/CamCtrl3D.h>
#include <Hd/Window.h>
#include <imgui.h>

namespace Hd {

void CamCtrl3D::fsfun(float x, float y)
{
    cam.setfbsize({ x, y });
    cam.setPersp();
}

void CamCtrl3D::mbutfun(int but, bool press)
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

    if (but == GLFW_MOUSE_BUTTON_MIDDLE && press) {
        rotate = !rotate;
        glfwSetInputMode(Hd::Window::getInstance().Id(), GLFW_CURSOR, rotate ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void CamCtrl3D::update()
{
    static glm::vec2 lastpos = { 0, 0 };
    glm::vec2 pos = Hd::Window::getInstance().getMousePos();
    glm::vec2 d;
    if (rotate) {
        d = pos - lastpos;
        // cam.rotate({ 5 * d.y, 0, 0 });
        cam.rotateXZ(d.x, d.y);
    }
    lastpos = lastpos + d * 0.2f;

    if (glfwGetKey(Hd::Window::getInstance().Id(), GLFW_KEY_W) == GLFW_PRESS) {
        cam.moveC({ 0, 0, -0.1 });
    }

    if (glfwGetKey(Hd::Window::getInstance().Id(), GLFW_KEY_S) == GLFW_PRESS) {
        cam.moveC({ 0, 0, 0.1 });
    }

    if (glfwGetKey(Hd::Window::getInstance().Id(), GLFW_KEY_A) == GLFW_PRESS) {
        cam.moveC({ 0.1, 0, 0 });
    }

    if (glfwGetKey(Hd::Window::getInstance().Id(), GLFW_KEY_D) == GLFW_PRESS) {
        cam.moveC({ -0.1, 0, 0 });
    }
}

void CamCtrl3D::mposfun(float, float)
{
    if (moving) {

        glm::vec2 diff = Hd::Window::getInstance().getMousePos() - movloc;

        cam.setPos(movpos);
        cam.setDir(movdir);
        cam.rotateA(diff, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    }
}

CamCtrl3D::CamCtrl3D(Hd::Camera& c)
    : CamCtrl(c)
{
    auto s = Hd::Window::getInstance().getFramebufferSize();
    fsfun(s.x, s.y);
}

void CamCtrl3D::gui()
{
    using namespace ImGui;

    static glm::vec3 m { 0.0f };
    static glm::vec3 r { 0.0f };
    static glm::vec3 mc { 0.0f };

    SliderFloat3("move", &m.x, -0.5f, 0.5f);
    if (IsItemActive()) {
        cam.move(m);
    } else {
        m = { 0, 0, 0 };
    }
    SliderFloat3("rotate", &r.x, -0.5f, 0.5f);
    if (IsItemActive()) {
        cam.rotate(r);
    } else {
        r = { 0, 0, 0 };
    }
    SliderFloat3("movc", &mc.x, -0.5f, 0.5f);
    if (IsItemActive()) {
        cam.moveC(mc);
    } else {
        mc = { 0, 0, 0 };
    }
}

}

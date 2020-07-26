#pragma once

#include <imgui.h>
#include <Hd/Window.h>

namespace Hd {

class Gui {
public:
    Gui(Hd::Window& w);
    ~Gui();
    void Draw();
    void GuiFunc();
};

}

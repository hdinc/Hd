#pragma once

#include <imgui.h>
#include <Hd/FunctionList.h>
#include <Hd/Window.h>

namespace Hd {

class Gui {
public:
    Gui(Hd::Window* w);
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    void Draw();
    FunctionList<void> Functions;
};

}

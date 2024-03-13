#pragma once

#include <imgui.h>
#include <Hd/FunctionList.h>

namespace Hd {

class Gui {
public:
    Gui();
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    void Draw();
    FunctionList<void> Functions;
};

}

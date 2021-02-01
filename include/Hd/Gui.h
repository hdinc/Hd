#pragma once

#include <imgui.h>
#include <Hd/FunctionList.h>

namespace Hd {

class Gui {
public:
    Gui();
    ~Gui();
    void Draw();
    FunctionList<void> Functions;
};

}

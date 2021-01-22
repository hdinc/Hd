#pragma once

#include <imgui.h>

namespace Hd {

typedef void (*guifunc)();

class Gui {
public:
    Gui();
    ~Gui();
    void Draw();

    // TODO: make a separate class to run multiple function pointers
    // or use stl
    // use that for camera2d and gui classes

    int addFunc(guifunc function);
    void removeFunc(int n);

private:
    static const int maxcount_ = 20;
    int mCount = 0;
    int mIdCount = 0;
    guifunc mFunctions[maxcount_] = { 0 };
    int mIds[maxcount_] = { 0 };
};

}

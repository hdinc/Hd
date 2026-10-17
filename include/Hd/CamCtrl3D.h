#pragma once

#include "CamCtrl.h"

namespace Hd {

class CamCtrl3D : public CamCtrl {
public:
    CamCtrl3D(Hd::Camera& cam);
    void gui() override;
    void update() override;

private:
    void fsfun(float x, float y) override;
    void mposfun(float x, float y) override;
    void mbutfun(int button, bool press) override;

    bool rotate = false;
};

}

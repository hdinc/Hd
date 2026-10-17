#pragma once

#include "CamCtrl.h"

namespace Hd {

class CamCtrl2D : public CamCtrl {
public:
    CamCtrl2D(Hd::Camera& cam);
    void gui() override;
    void update() override;

private:
    void fsfun(float x, float y) override;
    void mposfun(float x, float y) override;
    void mbutfun(int button, bool press) override;
};

}

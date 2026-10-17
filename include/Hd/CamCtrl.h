#pragma once

#include "Camera.h"

namespace Hd {

class CamCtrl {
public:
    CamCtrl(Hd::Camera& c);
    virtual ~CamCtrl();

    virtual void gui() = 0;

    // call once per frame for continuous input (e.g. keyboard movement)
    virtual void update() = 0;

    CamCtrl(const CamCtrl&) = delete;
    CamCtrl& operator=(const CamCtrl&) = delete;

protected:
    Hd::Camera& cam;

    int fscb;
    int mposcb;
    int mbutcb;

    bool moving = false;
    glm::vec2 movloc;
    glm::quat movdir;
    glm::vec3 movpos;

    virtual void fsfun(float x, float y) = 0;
    virtual void mposfun(float x, float y) = 0;
    virtual void mbutfun(int button, bool press) = 0;
};

}

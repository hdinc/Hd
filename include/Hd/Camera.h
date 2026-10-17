#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Hd {

class Camera {

public:
    Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    Camera(Camera&&) = default;

    const glm::mat4& getvp() { return vp; }

    void setOrtho();
    void setPersp(float fov = 45.0f);

    void rotate(glm::vec3 ang);
    void rotateXZ(float ax, float az);
    void move(glm::vec3 d);
    void rotateA(glm::vec2 ang, glm::vec3 pos, glm::vec3 up);
    void moveC(glm::vec3 d);

    void setPos(glm::vec3 pos);
    void setDir(glm::quat dir);
    void setDir(float angx, float angz);

    void setfbsize(glm::vec2 fb);

    glm::vec3 getPos() { return pos; }
    glm::quat getDir() { return dir; }

    friend class CamCtrl;

    enum ProjectionType {
        Perspective,
        Orthogonal
    };

private:
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 vp;

    glm::vec3 pos;
    glm::quat dir;

    glm::vec2 fbsize;

    ProjectionType ptype = Perspective;

    float fov = 45.0f; // degrees

    void recalcV();
    void recalcP();
    void recalcVP();
};

}

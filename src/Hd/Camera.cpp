#include <Hd/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hd {

Camera::Camera()
    : pos { 0.0f, -10.0f, 0.0f }
    , dir { 1.0f, 0.0f, 0.0f, 0.0f }
    , fbsize { 1.0f, 1.0f }
{
    setPersp();
    recalcV();
}

void Camera::recalcV()
{
    auto m = glm::mat4_cast(dir);
    view = glm::lookAt(pos, pos + glm::vec3(m[1]), glm::vec3(m[2]));

    // orthographic zoom depends on pos.z
    if (ptype == Orthogonal)
        recalcP();
    else
        recalcVP();
}

void Camera::recalcP()
{
    float aspect = fbsize.y / fbsize.x;
    if (ptype == Perspective) {
        projection = glm::perspective(glm::radians(fov), 1.0f / aspect, 0.1f, 1000.0f);
    } else {
        // pos.z is the half height of the visible area
        projection = glm::ortho(-pos.z / aspect, pos.z / aspect, -pos.z, pos.z, -1000.0f, 1000.0f);
    }

    recalcVP();
}

void Camera::recalcVP()
{
    vp = projection * view;
}

void Camera::setOrtho()
{
    ptype = Orthogonal;
    recalcP();
}

void Camera::setPersp(float fov)
{
    this->fov = fov;
    ptype = Perspective;
    recalcP();
}

void Camera::rotateXZ(float ax, float az)
{
    glm::mat4 d = glm::mat4_cast(dir);

    glm::mat4 m { 1.0f };

    m = glm::rotate(m, ax, { 0, 0, 1 });
    m = glm::rotate(m, az, glm::vec3(d[0]));

    dir *= glm::quat_cast(m);

    recalcV();
}

void Camera::rotate(glm::vec3 ang)
{
    dir *= glm::quat(glm::radians(ang));
    recalcV();
}

void Camera::rotateA(glm::vec2 ang, glm::vec3 p, glm::vec3 up)
{

    auto d = glm::normalize(pos - p);
    auto r = glm::cross(d, up);

    glm::mat4 m { 1.0f };
    m = glm::rotate(m, ang.x, up);
    m = glm::rotate(m, ang.y, r);

    dir = glm::quat(m) * dir;
    pos = glm::quat(m) * (pos - p) + p;

    recalcV();
}

void Camera::move(glm::vec3 d)
{
    pos += d;

    recalcV();
}

void Camera::moveC(glm::vec3 d)
{
    d = glm::vec3 { -d.x, -d.z, d.y };
    auto m = glm::mat4_cast(dir);
    pos += glm::vec3 { m * glm::vec4 { d, 1.0f } };
    recalcV();
}

void Camera::setPos(glm::vec3 p)
{
    pos = p;
    recalcV();
}

void Camera::setDir(glm::quat d)
{
    dir = d;
    recalcV();
}

void Camera::setDir(float angx, float angz)
{
    glm::mat4 m { 1.0f };
    m = glm::rotate(m, angx, { 1, 0, 0 });
    m = glm::rotate(m, angz, { 0, 0, 1 });

    dir = glm::quat_cast(m);

    recalcV();
}

void Camera::setfbsize(glm::vec2 fb)
{
    fbsize = fb;
    recalcP();
}

}

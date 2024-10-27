#pragma once

#include <glm/glm.hpp>

#include <Hd/Texture.h>
#include <Hd/Shader.h>
#include <Hd/Camera.h>

namespace Hd {

class Primitives {
public:
    Primitives();
    ~Primitives();

    void drawRect(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2);
    void drawTexturedRect(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2, Hd::Texture& t);
    void drawLine(Hd::Camera& c, glm::vec2 p1, glm::vec2 p2);

    void setColor(glm::vec4 c);

private:
    GLuint vao;

    Hd::Shader lineShader;
    Hd::Shader rectShader;
    Hd::Shader rectTShader;
};

}

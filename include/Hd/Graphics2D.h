#pragma once

#include <glm/glm.hpp>

#include <Hd/Texture.h>
#include <Hd/Shader.h>
#include <Hd/Camera.h>

namespace Hd {

class Graphics2D {
public:
    Graphics2D();
    ~Graphics2D();

    void rect        (const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, bool filled = false);
    void texturedRect(const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, Hd::Texture& t);
    void line        (const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2);
    void circle      (const glm::mat4& vp, glm::vec2 c, float r);

    void setColor(glm::vec4 c);

private:
    GLuint vao;

    Hd::Shader lineShader;
    Hd::Shader rectShader;
    Hd::Shader rectTShader;
    Hd::Shader circleShader;

    glm::vec4 color;
};

}

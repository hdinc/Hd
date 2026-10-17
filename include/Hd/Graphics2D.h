#pragma once

#include <glm/glm.hpp>

#include <Hd/Texture.h>
#include <Hd/Shader.h>
#include <Hd/Camera.h>
#include <Hd/VAO.h>
#include <Hd/Text.h>

namespace Hd {

class Graphics2D {
public:
    Graphics2D();
    ~Graphics2D();

    void rect        (const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, bool filled = false);
    void texturedRect(const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2, Hd::Texture& t);
    void line        (const glm::mat4& vp, glm::vec2 p1, glm::vec2 p2);
    void circle      (const glm::mat4& vp, glm::vec2 c, float r, bool filled = false);
    void text        (const glm::mat4& vp, glm::vec2 p, float height);

    void setColor(glm::vec4 c);
    void setText(std::string text);
    std::pair<glm::vec2, glm::vec2> getTextBox();

    private:
    VAO vao;

    Hd::Shader lineShader;
    Hd::Shader rectShader;
    Hd::Shader rectTShader;
    Hd::Shader circleShader;
    Hd::Shader circleFilledShader;

    Hd::Text t;

    glm::vec4 color;

};

}

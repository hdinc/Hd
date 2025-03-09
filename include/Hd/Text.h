#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <Hd/Shader.h>
#include <Hd/Texture.h>
#include <Hd/VAO.h>
#include <Hd/VBO.h>
#include <stb/stb_truetype.h>

namespace Hd {

class Text {
public:
    Text(int pixel_size, const char* font=0);
    ~Text();

    enum class align {
        left,
        center,
        right
    };

    void set_text(std::string t);
    void draw(glm::mat4 vp, glm::vec2 fbsize, glm::vec2 fbscale, glm::vec2 pos, float height, glm::vec3 color = {0,0,1}, align align = align::left);


    glm::vec2 box;

private:
    Hd::Shader shader;
    GLuint font_texture;
    Hd::VBO vbo;
    Hd::VAO vao;

    int size;

    static constexpr int font_texture_size = 1024;
    stbtt_bakedchar cdata[96]; // ASCII [32,126] - total 95 glyphs

    std::string text;

    std::vector<std::array<float, 8>> uniforms;
    std::vector<glm::vec2> dlocs;
    std::vector<glm::vec2> sizes;

};

}

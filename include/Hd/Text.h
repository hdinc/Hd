#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <Hd/Shader.h>
#include <Hd/Texture.h>
#include <stb/stb_truetype.h>

namespace Hd {

class Text {
public:
    Text(int pixel_size, const char* font=0);
    ~Text();
    void update(std::string text);
    void draw(glm::vec2 fb_size, glm::vec2 pos);
    void set_color(glm::vec3 c);
    glm::vec2 getSize();

private:
    Hd::Shader shader;
    GLuint font_texture;
    GLuint vao, vbo;

    int size;

    static constexpr int font_texture_size = 1024;
    stbtt_bakedchar cdata[96]; // ASCII [32,126] - total 95 glyphs

    std::string buffer;

    std::vector<std::array<float, 8>> uniforms;
    std::vector<glm::vec2> dlocs;
    std::vector<glm::vec2> sizes;

    glm::vec3 color = glm::vec3{1.0f};

    glm::vec2 mbufsize;
};

}

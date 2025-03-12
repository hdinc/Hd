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

    void set_text(std::string t);
    void draw(glm::mat4 vp, glm::vec2 pos, float height, glm::vec3 color = {1,1,1});
    std::pair<glm::vec2,glm::vec2> get_box();


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

    std::pair<glm::vec2, glm::vec2> box;

};

}

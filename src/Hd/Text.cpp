#include <Hd/Text.h>
#include <Hd/Resource.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb/stb_truetype.h>
#include <array>

namespace Hd {

Text::Text(int size, const char* font)
    : shader(Resource::getLibRes("res/text/text_vertex.glsl").first, Resource::getLibRes("res/text/text_texture.glsl").first)
    , size(size)
{

    box = {{0, 0}, {0, 0}};

    // clang-format off
    float square[] = {
        -0.5f,  0.5f,
         0.5f,  0.5f,
        -0.5f, -0.5f,
         0.5f, -0.5f,
        };
    // clang-format on

    vao.Bind();
    vbo.Bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    unsigned char temp_bitmap[font_texture_size * font_texture_size];

    if (font == 0) {
        font = Hd::Resource::getLibRes("res/fonts/ElenaWebRegular.ttf").first;
    }

    stbtt_BakeFontBitmap((const unsigned char*)font, 0, size, temp_bitmap, font_texture_size, font_texture_size, 32, 96, cdata);

    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font_texture_size, font_texture_size, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Text::~Text()
{
    glDeleteTextures(1, &font_texture);
}

void Text::draw(glm::mat4 vp, glm::vec2 pos, float height, glm::vec3 color)
{
    shader.Bind();
    glUniform3fv(6, 1, &color.x);

    for (auto i = 0ul; i < text.size(); i++) {

        glm::vec2 dsize = sizes[i] * height;
        glm::vec2 dloc = pos + dlocs[i] * height;

        glm::mat4 m(1.0f);
        m = glm::translate(m, { dloc, 0.0f });
        m = glm::scale(m, { dsize, 1.0f });

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        vao.Bind();

        glUniform2fv(2, 4, &uniforms[i][0]);
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(vp * m));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Text::set_text(std::string t)
{
    text = std::move(t);

    dlocs.clear();
    sizes.clear();
    uniforms.clear();

    float cposx = 0;
    float cposy = 0;

    box = {{0, 0}, {0, 0}};

    for (auto i = 0ul; i < text.size(); i++) {

        unsigned char c = text[i];

        int ix;

        if (c >= 32 && c <= 128) {
            ix = (c - 32);
        } else if (c == '\n') {
            cposx = 0;
            cposy += size;
            continue;
        } else {
            ix = 0;
        }

        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, font_texture_size, font_texture_size, ix, &cposx, &cposy, &q, 1);

        std::array<float, 8> uniform;
        uniform[0] = q.s0;
        uniform[1] = q.t1;
        uniform[2] = q.s1;
        uniform[3] = q.t1;
        uniform[4] = q.s0;
        uniform[5] = q.t0;
        uniform[6] = q.s1;
        uniform[7] = q.t0;

        uniforms.push_back(uniform);
        dlocs.push_back(glm::vec2((q.x1 + q.x0) / (2.0f * float(size)), -(q.y1 + q.y0) / (2.0f * float(size))));
        sizes.push_back(glm::vec2((q.x1 - q.x0) / float(size), (q.y0 - q.y1) / float(size)));

        if (i == 0) {
            box.first = {q.x0, -q.y0};
            box.second = {q.x1, -q.y1};
        } else {
            if ( q.x0 < box.first.x) box.first.x  =  q.x0;
            if (-q.y0 > box.first.y) box.first.y  = -q.y0;
            if ( q.x1 > box.second.x) box.second.x =  q.x1;
            if (-q.y1 < box.second.y) box.second.y = -q.y1;
        }

    }

    box.first /= float(size);
    box.second /= float(size);

}

std::pair<glm::vec2, glm::vec2> Text::get_box() {
    return box;
}

}

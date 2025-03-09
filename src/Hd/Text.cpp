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
    , vbo(GL_ARRAY_BUFFER)
    , size(size)
{

    // clang-format off
    float square[] = {
        -1.f,  1.f,
         1.f,  1.f,
        -1.f, -1.f,
         1.f, -1.f,
        };
    // clang-format on

    vao.Bind();
    vbo.Data(square, sizeof(square));
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

void Text::draw(glm::mat4 vp, glm::vec2 fbsize, glm::vec2 fbscale, glm::vec2 pos, float height, glm::vec3 color, align align)
{
    shader.Bind();
    glUniform3fv(6, 1, &color.x);

    switch (align) {
    case align::center:
        pos -= 2.0f * box / (fbsize * fbscale);
        break;
    case align::left:
        break;
    case align::right:
        pos.x -= 2.0f * box.x / (fbsize * fbscale).x;
        break;
    }

    for (auto i = 0ul; i < text.size(); i++) {

        glm::vec2 size = (sizes[i] / (fbsize * fbscale)) / height;
        glm::vec2 dloc = pos + dlocs[i] / (fbsize * fbscale);

        glm::mat4 m(1.0f);
        m = glm::translate(m, { dloc, 0.0f });
        m = glm::scale(m, { size, 1.0f });

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

    box = glm::vec2(0);

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
        uniform[1] = q.t0;
        uniform[2] = q.s1;
        uniform[3] = q.t0;
        uniform[4] = q.s0;
        uniform[5] = q.t1;
        uniform[6] = q.s1;
        uniform[7] = q.t1;

        uniforms.push_back(uniform);
        dlocs.push_back(glm::vec2(q.x1 + q.x0, -q.y1 - q.y0));
        sizes.push_back(glm::vec2(q.x1 - q.x0, q.y1 - q.y0));

        box.x = cposx > box.x ? cposx : box.x;
        box.y = cposy + size;
    }

}

}

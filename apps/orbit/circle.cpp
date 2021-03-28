#include "circle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLuint circle::vao = 0;
GLuint circle::vbo = 0;
int circle::vcount = 128;

glm::mat4* circle::viewport = 0;
glm::mat4* circle::projection = 0;
Hd::Shader* circle::shader = 0;

GLint circle::MVP = 0;
GLint circle::COLOR = 0;

void circle::init_gldata(Hd::Shader* s, glm::mat4* v, glm::mat4* p)
{

    shader = s;
    viewport = v;
    projection = p;

    MVP = glGetUniformLocation(shader->Id(), "u_mvp");
    COLOR = glGetUniformLocation(shader->Id(), "u_color");
    shader->Bind();
    glUniform4f(COLOR, 1, 1, 1, 1);

    auto vertices = new float[vcount][2];
    for (int i = 0; i < vcount; i++) {
        vertices[i][0] = cos(i * M_PI / ((float)vcount / 2.0f));
        vertices[i][1] = sin(i * M_PI / ((float)vcount / 2.0f));
    }

    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vcount * 2 * sizeof(float), vertices,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    delete[] vertices;
}

void circle::destroy_gldata()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void circle::draw()
{
    glBindVertexArray(vao);
    mvp = *projection * *viewport * model;
    shader->Bind();
    glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4f(COLOR, color.x, color.y, color.z, 1);
    glDrawArrays(GL_LINE_LOOP, 0, vcount);
}

circle::circle(float r, glm::vec2 loc)
{
    radius = r;
    location = loc;
    color = glm::vec3(1);

    model = glm::mat4(1);
    model = glm::translate(model, glm::vec3(loc, 0.0f));
    model = glm::scale(model, glm::vec3(radius));
}

circle::~circle() { }

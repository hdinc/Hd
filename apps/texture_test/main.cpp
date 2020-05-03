#include <Hd.h>

int main()
{
    Hd::Window window("test", 500, 500);
    Hd::Shader shader("../res/shaders/vertex.glsl","../res/shaders/texture.glsl");
    Hd::Texture texture("../res/textures/diyojen.png");

    float square[] = {
        -1, -1, 0, 0,
        -1, 1, 0, 1,
        1, -1, 1, 0,
        1, 1, 1, 1
    };

    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    texture.Bind(0);

    glClearColor(.0f, .0f, .0f, .0f);
    shader.Bind();
    glUniform1i(glGetUniformLocation(shader.m_ProgramId,"tex"),0);

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        window.SwapBuffers();
        window.PollEvents();
    }
}

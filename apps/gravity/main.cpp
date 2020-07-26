#include <Hd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

struct point {
    float location[2][2];
    float speed[2];
};

void initpoints(point* points, int count);

int main()
{

    srand(time(0));
    Hd::Window window("gravity", 1280, 720);
    point points[240];
    initpoints(points, 240);

    GLuint pointbuf;
    glGenBuffers(1, &pointbuf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointbuf);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(points), points, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointbuf);

    glBindBuffer(GL_ARRAY_BUFFER, pointbuf);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(point), (void*)(2 * sizeof(float)));

    Hd::Shader shader("vertex.glsl", "frag.glsl");
    shader.Bind();

    while (!window.ShouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINT, 0, 240);
        window.SwapBuffers();
        window.PollEvents();
    }

    return 0;
}

void initpoints(point* points, int count)
{
    for (int i = 0; i < count; i++) {
        points[i].location[0][0] = i % 10 - 5;
        points[i].location[0][1] = i / 10 - 5;
    }
}

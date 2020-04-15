#include <Hd.h>

int main()
{
    Hd::Window window("test", 500, 500);

    glClearColor(.3f, .6f, .4f, 1.f);

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        window.SwapBuffers();
        window.PollEvents();
    }
}

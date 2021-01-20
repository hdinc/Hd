#include <Hd.h>

int main()
{
    Hd::Window::setName("test");
    Hd::Window::setSize(500, 500);
    auto& window = Hd::Window::getInstance();

    glClearColor(.3f, .6f, .4f, 1.f);

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);

        window.SwapBuffers();
        window.PollEvents();
    }
}

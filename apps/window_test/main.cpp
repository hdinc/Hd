#include <Hd.h>

int main()
{
    Hd::Window::setName("test");
    Hd::Window::setSize(500, 500);
    Hd::Gui g;
    g.Functions.add(std::bind(ImGui::ShowDemoWindow, nullptr));
    auto& window = Hd::Window::getInstance();

    glClearColor(.3f, .6f, .4f, 1.f);

    while (!window.ShouldClose()) {

        glClear(GL_COLOR_BUFFER_BIT);
        g.Draw();

        window.SwapBuffers();
        window.PollEvents();
    }
}

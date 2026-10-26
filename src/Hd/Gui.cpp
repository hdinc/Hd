#include <Hd/Gui.h>
#include <Hd/Window.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Hd {
Gui::Gui(Hd::Window* w)
{
    // TODO: only run first instance
    // https://stackoverflow.com/questions/17058701/member-function-called-only-on-initialization-of-first-instance-of-a-class-c/17058756

    // TODO: make gui singleton

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(w->Id(), true);
#ifdef __EMSCRIPTEN__
    // the canvas follows the size of the browser window
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(w->Id(), "#canvas");
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
}

Gui::~Gui()
{

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::Draw()
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Functions.run();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
}

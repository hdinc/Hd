#include <Hd/Gui.h>
#include <Hd/Window.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdio.h>

namespace Hd {
Gui::Gui()
{
    // TODO: only run first instance
    // https://stackoverflow.com/questions/17058701/member-function-called-only-on-initialization-of-first-instance-of-a-class-c/17058756

    // TODO: make gui singleton

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(Window::getInstance().Id(), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    Window::getInstance().hasGui = true;
}

Gui::~Gui()
{

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Window::getInstance().hasGui = false;
}

int Gui::addFunc(guifunc function)
{
    if (mCount >= maxcount_) {
        printf("[GUI]:max guifunc count exceeded\n");
        return -1;
    }
    mFunctions[mCount] = function;
    mIds[mCount] = ++mIdCount;
    mCount++;
    return mIdCount;
}

void Gui::removeFunc(int n)
{
    for (int i = 0; i < mCount; i++) {
        if (mIds[i] == n) {
            mFunctions[i] = mFunctions[mCount];
            mIds[i] = mIds[mCount];
            mCount--;
            return;
        }
    }
    printf("[GUI]:invalid guifunc id\n");
}

void Gui::Draw()
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (int i = 0; i < mCount; i++) {
        mFunctions[i]();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
}

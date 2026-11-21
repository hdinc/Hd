#include <Hd/Gui.h>
#include <Hd/Platform.h>
#include <Hd/Theme.h>
#include <Hd/Window.h>

#include <algorithm>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Hd {

Gui::Gui(Hd::Window* w)
    : mWindow(w)
{
    // TODO: only run first instance
    // https://stackoverflow.com/questions/17058701/member-function-called-only-on-initialization-of-first-instance-of-a-class-c/17058756

    // TODO: make gui singleton

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;

    ImGui_ImplGlfw_InitForOpenGL(w->Id(), true);
#ifdef __EMSCRIPTEN__
    // the canvas follows the size of the browser window
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(w->Id(), "#canvas");
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif

    mDpi = w->getContentScale();
    mScaleCb = w->ContentScaleCb.add([this](float s) {
        mDpi = s;
        mStyleDirty = true;
    });
    // the look is applied before the first frame: the system's font and
    // the Dark theme unless the program picks others first
    restyle();
}

Gui::~Gui()
{
    mWindow->ContentScaleCb.remove(mScaleCb);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::setUserScale(float s)
{
    s = std::clamp(s, 0.5f, 3.0f);
    if (s == mUserScale) return;
    mUserScale = s;
    mStyleDirty = true;
}

void Gui::setFonts(const FontRef& ui, const FontRef& mono)
{
    mUi = ui;
    mMono = mono;
    mFontsDirty = true;
}

void Gui::setFontSize(float size)
{
    size = std::clamp(size, 6.0f, 72.0f);
    if (size == mFontSize) return;
    mFontSize = size;
    mStyleDirty = true; // fonts are sized by the style: nothing to load
}

void Gui::setTheme(const std::string& key)
{
    mTheme = key;
    mThemeDirty = true;
}

bool Gui::lightTheme() const { return Theme::current().base == "light"; }

void Gui::restyle()
{
    if (mFontsDirty) {
        mFontsDirty = false;
        mFontErrors.clear();
        FontRef ui = mUi;
        if (ui.builtin())
            if (std::string sys = Platform::systemUiFont(); !sys.empty()) ui.file = std::filesystem::path(std::u8string(sys.begin(), sys.end()));
        loadFonts(ui, mMono, {}, mFontErrors);
    }
    if (mThemeDirty) {
        mThemeDirty = false;
        if (mTheme == "system") mResolved = Platform::prefersDarkTheme() ? "dark" : "light";
        Theme t;
        std::string error;
        Theme::find(mTheme == "system" ? mResolved : mTheme, t, error); // Dark when it can't be found
        Theme::setCurrent(std::move(t));
        mStyleDirty = true;
    }
    if (!mStyleDirty) return;
    mStyleDirty = false;
    // sizes from ImGui's defaults, the theme's colours and shapes, then all
    // of it scaled; fonts are drawn at the scaled size
    ImGuiStyle& st = ImGui::GetStyle();
    st = ImGuiStyle();
    Theme::applyCurrent();
    Hd::setFontSize(mFontSize);
    st.ScaleAllSizes(scale());
    st.FontScaleDpi = scale();
    mWindow->setDarkTitleBar(!lightTheme());
}

void Gui::Draw()
{
    // the look changes between frames, never during one
    if (mTheme == "system" && ImGui::GetTime() - mLastSystemLook > 2) {
        mLastSystemLook = ImGui::GetTime();
        if ((Platform::prefersDarkTheme() ? "dark" : "light") != mResolved) mThemeDirty = true;
    }
    restyle();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Functions.run();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::waitEvents(bool busy, double timeout)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseDown[0] || io.MouseDown[1] || io.MouseDown[2]) mExtra = 3;
    if (busy || mExtra > 0) {
        mWindow->PollEvents();
        mExtra = std::max(0, mExtra - 1);
        return;
    }
    if (timeout > 0)
        mWindow->WaitEventsTimeout(timeout);
    else
        mWindow->WaitEvents();
    mExtra = 3;
}

}

#pragma once

#include <string>
#include <vector>

#include <imgui.h>
#include <Hd/Fonts.h>
#include <Hd/FunctionList.h>
#include <Hd/Window.h>

namespace Hd {

// ImGui on a Hd window: the frame, and the look. Sizes follow the monitor's
// scale (and move with the window between monitors) times a scale the user
// may set. Fonts and themes are those of Fonts.h and Theme.h; changes to
// them are applied between frames.
class Gui {
public:
    Gui(Hd::Window* w);
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    void Draw();
    FunctionList<void> Functions;

    // --- size
    // the whole UI's scale: the monitor's times the user's
    float scale() const { return mDpi * mUserScale; }
    float dpiScale() const { return mDpi; }
    float userScale() const { return mUserScale; }
    // 0.5 to 3
    void setUserScale(float s);

    // --- fonts (see Fonts.h). A built-in ui font means the system's
    // interface font (Segoe UI, fontconfig's sans-serif), Hd's own when
    // there is none; a built-in mono font the system's monospace one.
    void setFonts(const FontRef& ui, const FontRef& mono = {});
    const FontRef& uiFont() const { return mUi; }
    const FontRef& monoFont() const { return mMono; }
    // text height in pixels at scale 1
    void setFontSize(float size);
    float fontSize() const { return mFontSize; }
    // The UI font's bold face, if available. Refresh after font changes.
    ImFont* boldFont() const { return mBold; }
    // why the last fonts couldn't be loaded, if they couldn't
    const std::vector<std::string>& fontErrors() const { return mFontErrors; }

    // --- theme (see Theme.h): a key Theme::find takes ("dark", "light",
    // "black", a custom theme's), or "system" for dark or light as the
    // operating system is set, followed while the program runs
    void setTheme(const std::string& key);
    const std::string& theme() const { return mTheme; }
    // the theme in use has a light background
    bool lightTheme() const;
    // applies Theme::current() again, after it was edited (ThemeEditor's
    // useCurrent)
    void refreshStyle() { mStyleDirty = true; }

    // --- frames
    // waits for events unless busy, then lets a few frames be drawn per
    // wake: ImGui takes queued input a step per frame (a click is a press,
    // then a release). timeout > 0 wakes up after that many seconds anyway.
    void waitEvents(bool busy, double timeout = -1);

private:
    void restyle();

    Hd::Window* mWindow;
    int mScaleCb = 0;
    float mDpi = 1;
    float mUserScale = 1;
    std::string mTheme = "dark";
    std::string mResolved; // the key "system" stood for at the last look
    double mLastSystemLook = 0;
    FontRef mUi, mMono;
    float mFontSize = 16;
    std::vector<std::string> mFontErrors;
    ImFont* mBold = nullptr;
    bool mFontsDirty = true;
    bool mThemeDirty = true;
    bool mStyleDirty = true;
    int mExtra = 0;
};

}

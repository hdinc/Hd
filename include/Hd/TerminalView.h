#pragma once

// a terminal in an imgui window: the screen and history of a Vt100, keys
// typed into it, copy and paste

#include <Hd/Vt100.h>

#include <imgui.h>

#include <optional>
#include <string>
#include <utility>

namespace Hd {

class TerminalView {
public:
    Vt100 vt;
    // the screen's size from the window's, else 80 x 24
    bool fit = false;

    // a buttons line (paste, copy, clear, fit) and the screen in the rest of
    // the current window. while it has focus the keys are the terminal's,
    // not the app's shortcuts'. returns the bytes typed or pasted, for the
    // other side. a window with ImGuiWindowFlags_NoNavInputs keeps the
    // arrows and tab.
    std::string gui();

    // the state after power on, no history and no selection
    void reset();

    // a window size for 80 x 24 with the buttons, in the mono font
    static ImVec2 windowSize();

private:
    std::string selectionText() const;
    std::string pasted();

    // the view stays at the bottom while new text comes
    bool follow = true;
    // a selection from the mouse, (line, column) of Vt100::lineAt
    bool selecting = false;
    std::optional<std::pair<size_t, int>> selFrom, selTo;
};

}

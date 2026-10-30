#include <Hd/TerminalView.h>
#include <Hd/Fonts.h>
#include <Hd/Icons.h>

#include <imgui_internal.h>

#include <algorithm>

namespace Hd {

namespace {

// the bytes a key sends, empty for keys that send nothing
std::string keyBytes(ImGuiKey k, bool appCursor)
{
    auto cursor = [&](char c) { return std::string(appCursor ? "\x1bO" : "\x1b[") + c; };
    switch (k) {
    case ImGuiKey_Enter:
    case ImGuiKey_KeypadEnter: return "\r";
    case ImGuiKey_Backspace: return "\x7f";
    case ImGuiKey_Tab: return "\t";
    case ImGuiKey_Escape: return "\x1b";
    case ImGuiKey_UpArrow: return cursor('A');
    case ImGuiKey_DownArrow: return cursor('B');
    case ImGuiKey_RightArrow: return cursor('C');
    case ImGuiKey_LeftArrow: return cursor('D');
    case ImGuiKey_Home: return cursor('H');
    case ImGuiKey_End: return cursor('F');
    case ImGuiKey_Insert: return "\x1b[2~";
    case ImGuiKey_Delete: return "\x1b[3~";
    case ImGuiKey_PageUp: return "\x1b[5~";
    case ImGuiKey_PageDown: return "\x1b[6~";
    case ImGuiKey_F1: return "\x1bOP";
    case ImGuiKey_F2: return "\x1bOQ";
    case ImGuiKey_F3: return "\x1bOR";
    case ImGuiKey_F4: return "\x1bOS";
    case ImGuiKey_F5: return "\x1b[15~";
    case ImGuiKey_F6: return "\x1b[17~";
    case ImGuiKey_F7: return "\x1b[18~";
    case ImGuiKey_F8: return "\x1b[19~";
    case ImGuiKey_F9: return "\x1b[20~";
    case ImGuiKey_F10: return "\x1b[21~";
    case ImGuiKey_F11: return "\x1b[23~";
    case ImGuiKey_F12: return "\x1b[24~";
    default: return {};
    }
}

void appendUtf8(std::string& s, unsigned c)
{
    if (c < 0x80) {
        s += char(c);
    } else if (c < 0x800) {
        s += char(0xC0 | (c >> 6));
        s += char(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        s += char(0xE0 | (c >> 12));
        s += char(0x80 | ((c >> 6) & 0x3F));
        s += char(0x80 | (c & 0x3F));
    } else {
        s += char(0xF0 | (c >> 18));
        s += char(0x80 | ((c >> 12) & 0x3F));
        s += char(0x80 | ((c >> 6) & 0x3F));
        s += char(0x80 | (c & 0x3F));
    }
}

ImU32 rgb(Vt100::Color c) { return IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255); }

}

void TerminalView::reset()
{
    vt.reset();
    selFrom.reset();
    selTo.reset();
    selecting = false;
}

ImVec2 TerminalView::windowSize()
{
    using namespace ImGui;
    MonoScope mono;
    return { CalcTextSize("M").x * 80 + 40, GetTextLineHeight() * 24 + GetFrameHeightWithSpacing() * 2 + 30 };
}

std::string TerminalView::selectionText() const
{
    if (!selFrom || !selTo) return {};
    auto a = *selFrom, b = *selTo;
    if (b < a) std::swap(a, b);
    return vt.text(a.first, a.second, b.first, b.second);
}

std::string TerminalView::pasted()
{
    const char* clip = ImGui::GetClipboardText();
    std::string bytes;
    if (!clip) return bytes;
    for (const char* p = clip; *p; p++)
        if (*p != '\r') bytes += *p == '\n' ? '\r' : *p;
    follow = true;
    return bytes;
}

std::string TerminalView::gui()
{
    using namespace ImGui;

    std::string out;
    float charW, lineH;
    {
        MonoScope mono;
        charW = CalcTextSize("M").x;
        lineH = GetTextLineHeight();
    }

    if (Button((std::string(Icon::paste) + " Paste").c_str())) out += pasted();
    SetItemTooltip("Types the clipboard's text (also Shift+Insert or Ctrl+Shift+V)");
    SameLine();
    BeginDisabled(!selFrom);
    if (Button((std::string(Icon::copy) + " Copy").c_str())) SetClipboardText(selectionText().c_str());
    SetItemTooltip("Copies the selection (also Ctrl+Shift+C)");
    EndDisabled();
    SameLine();
    if (Button((std::string(Icon::eraser) + " Clear").c_str())) {
        vt.reset();
        selFrom.reset();
    }
    SetItemTooltip("Clears the screen and its history here, the program doesn't see it");
    SameLine();
    Checkbox("Fit window", &fit);
    SetItemTooltip("The screen's size follows the window, else 80 x 24. Programs don't learn the size over a serial line, tell them with stty rows and cols.");
    SameLine();
    TextDisabled("%d x %d", vt.cols(), vt.rows());

    BeginChild("screen", { 0, 0 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_HorizontalScrollbar);
    // popped before EndChild
    std::optional<MonoScope> mono;
    mono.emplace();
    if (fit) {
        auto avail = GetContentRegionAvail();
        vt.resize(std::max(2, int(avail.x / charW)), std::max(2, int(avail.y / lineH)));
    } else if (vt.cols() != 80 || vt.rows() != 24) {
        vt.resize(80, 24);
    }

    bool focused = IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    ImGuiID owner = GetID("keys");
    auto& io = GetIO();
    if (focused) {
        // the keys are the terminal's while it has focus, not the app's
        // shortcuts'
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_GamepadStart; k++)
            SetKeyOwner((ImGuiKey)k, owner, ImGuiInputFlags_LockUntilRelease);
        std::string bytes;
        bool ctrl = io.KeyCtrl && !io.KeyAlt;
        if (!ctrl)
            for (auto c : io.InputQueueCharacters)
                appendUtf8(bytes, c);
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_GamepadStart; k++) {
            auto key = (ImGuiKey)k;
            if (!IsKeyPressed(key, ImGuiInputFlags_Repeat, owner)) continue;
            if (io.KeyShift && (key == ImGuiKey_PageUp || key == ImGuiKey_PageDown)) {
                SetScrollY(GetScrollY() + (key == ImGuiKey_PageUp ? -1 : 1) * lineH * float(vt.rows() - 1));
                follow = false;
                continue;
            }
            if ((io.KeyShift && key == ImGuiKey_Insert) || (ctrl && io.KeyShift && key == ImGuiKey_V)) {
                out += bytes;
                bytes.clear();
                out += pasted();
                continue;
            }
            if (ctrl && io.KeyShift && key == ImGuiKey_C) {
                SetClipboardText(selectionText().c_str());
                continue;
            }
            if (ctrl && key >= ImGuiKey_A && key <= ImGuiKey_Z) {
                bytes += char(1 + (key - ImGuiKey_A));
                continue;
            }
            if (ctrl && (key == ImGuiKey_LeftBracket || key == ImGuiKey_Backslash || key == ImGuiKey_RightBracket || key == ImGuiKey_Space)) {
                bytes += key == ImGuiKey_LeftBracket ? '\x1b' : key == ImGuiKey_Backslash ? '\x1c' : key == ImGuiKey_RightBracket ? '\x1d' : '\0';
                continue;
            }
            bytes += keyBytes(key, vt.applicationCursor());
        }
        if (!bytes.empty()) {
            out += bytes;
            follow = true;
        }
    }

    // the lines of the history and the screen, drawn as runs of one style
    size_t lines = vt.lines();
    auto origin = GetCursorScreenPos();
    float width = charW * float(vt.cols());
    Dummy({ width, lineH * float(lines) });
    auto* dl = GetWindowDrawList();
    ImU32 defaultFg = GetColorU32(ImGuiCol_Text);
    // opaque, the window's background may be translucent
    ImU32 defaultBg = GetColorU32(ImGuiCol_WindowBg) | IM_COL32_A_MASK;
    ImU32 selection = GetColorU32(ImGuiCol_TextSelectedBg);
    auto selA = selFrom, selB = selTo;
    if (selA && selB && *selB < *selA) std::swap(selA, selB);
    auto selected = [&](size_t l, int c) {
        if (!selA || !selB) return false;
        auto p = std::make_pair(l, c);
        return !(p < *selA) && !(*selB < p);
    };
    float scroll = GetScrollY(), viewH = GetWindowHeight();
    size_t first = size_t(std::max(0.0f, scroll / lineH)), last = std::min(lines, size_t((scroll + viewH) / lineH) + 2);
    dl->AddRectFilled(origin, { origin.x + width, origin.y + lineH * float(lines) }, defaultBg);
    for (size_t l = first; l < last; l++) {
        auto& row = vt.lineAt(l);
        float y = origin.y + lineH * float(l);
        for (int c = 0; c < (int)row.size();) {
            auto& cell = row[size_t(c)];
            bool sel = selected(l, c);
            // a run of cells with the same style
            int end = c + 1;
            while (end < (int)row.size() && row[size_t(end)].fg == cell.fg && row[size_t(end)].bg == cell.bg && row[size_t(end)].attrs == cell.attrs && selected(l, end) == sel)
                end++;
            ImU32 fg = cell.fg ? rgb(cell.fg) : defaultFg, bg = cell.bg ? rgb(cell.bg) : defaultBg;
            if (cell.attrs & Vt100::inverse) std::swap(fg, bg);
            if (sel) bg = selection;
            if (cell.attrs & Vt100::dim) fg = (fg & 0x00FFFFFF) | 0x99000000;
            float x0 = origin.x + charW * float(c), x1 = origin.x + charW * float(end);
            if (bg != defaultBg) dl->AddRectFilled({ x0, y }, { x1, y + lineH }, bg);
            std::string text;
            for (int k = c; k < end; k++)
                appendUtf8(text, row[size_t(k)].ch);
            dl->AddText({ x0, y }, fg, text.c_str());
            if (cell.attrs & Vt100::bold) dl->AddText({ x0 + 1, y }, fg, text.c_str());
            if (cell.attrs & Vt100::underline) dl->AddLine({ x0, y + lineH - 1 }, { x1, y + lineH - 1 }, fg);
            c = end;
        }
    }
    // the cursor, a block while focused
    if (vt.cursorVisible()) {
        float x = origin.x + charW * float(vt.cursorCol()), y = origin.y + lineH * float(vt.historySize() + size_t(vt.cursorRow()));
        if (focused)
            dl->AddRectFilled({ x, y }, { x + charW, y + lineH }, (defaultFg & 0x00FFFFFF) | 0x90000000);
        else
            dl->AddRect({ x, y }, { x + charW, y + lineH }, defaultFg);
    }

    // selecting with the mouse
    auto cellAt = [&](ImVec2 m) {
        long l = long((m.y - origin.y) / lineH);
        int c = int((m.x - origin.x) / charW);
        l = std::clamp(l, 0L, long(lines) - 1);
        return std::make_pair(size_t(l), std::clamp(c, 0, vt.cols() - 1));
    };
    if (IsWindowHovered() && IsMouseClicked(ImGuiMouseButton_Left)) {
        selFrom = selTo = cellAt(GetMousePos());
        selecting = true;
    }
    if (selecting) {
        if (IsMouseDown(ImGuiMouseButton_Left)) {
            selTo = cellAt(GetMousePos());
        } else {
            selecting = false;
            // a click without dragging selects nothing
            if (selFrom == selTo) selFrom.reset(), selTo.reset();
        }
    }
    if (BeginPopupContextWindow("terminal menu")) {
        if (MenuItem("Copy", "Ctrl+Shift+C", false, selFrom.has_value())) SetClipboardText(selectionText().c_str());
        if (MenuItem("Copy all")) SetClipboardText(vt.text(0, lines - 1).c_str());
        if (MenuItem("Paste", "Shift+Insert")) out += pasted();
        EndPopup();
    }

    // following the output unless scrolled up
    if (GetIO().MouseWheel != 0 && IsWindowHovered()) follow = false;
    if (GetScrollY() >= GetScrollMaxY() - 1) follow = true;
    if (follow) SetScrollY(GetScrollMaxY());
    mono.reset();
    EndChild();
    return out;
}

}

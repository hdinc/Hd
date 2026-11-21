#include <Hd/Icons.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

namespace Hd {

namespace {

// the box of the glyph's visible pixels, relative to where the text is
// drawn. the glyph's quad comes from the outline's control points, which
// stick out past curves (e.g. the arc of undo), so centering the quad leaves
// such icons a pixel off. the atlas keeps its pixels in memory, a glyph is a
// few hundred of them.
ImRect inkBox(const ImFontGlyph& g)
{
    ImRect quad{g.X0, g.Y0, g.X1, g.Y1};
    auto tex = ImGui::GetIO().Fonts->TexData;
    if (!tex || !tex->Pixels) return quad;
    int x0 = (int)ImFloor(g.U0 * tex->Width + 0.5f), x1 = (int)ImFloor(g.U1 * tex->Width + 0.5f);
    int y0 = (int)ImFloor(g.V0 * tex->Height + 0.5f), y1 = (int)ImFloor(g.V1 * tex->Height + 0.5f);
    if (x1 <= x0 || y1 <= y0 || x1 > tex->Width || y1 > tex->Height) return quad;

    // half covered or more counts as drawn, the eye sees the edge there
    int minx = x1, maxx = x0 - 1, miny = y1, maxy = y0 - 1;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            auto p = tex->Pixels + (size_t(y) * tex->Width + x) * tex->BytesPerPixel;
            int alpha = tex->Format == ImTextureFormat_Alpha8 ? p[0] : p[3];
            if (alpha < 128) continue;
            minx = ImMin(minx, x), maxx = ImMax(maxx, x);
            miny = ImMin(miny, y), maxy = ImMax(maxy, y);
        }
    }
    if (maxx < minx) return quad;
    // atlas pixels to the quad's coordinates
    float sx = (g.X1 - g.X0) / float(x1 - x0), sy = (g.Y1 - g.Y0) / float(y1 - y0);
    return {g.X0 + (minx - x0) * sx, g.Y0 + (miny - y0) * sy, g.X0 + (maxx + 1 - x0) * sx, g.Y0 + (maxy + 1 - y0) * sy};
}

}

namespace {

ToolbarSize g_size = ToolbarSize::small;

// the icon's size near `want` (its font's em) at which it fits a w x h box
// with the same space on both sides: its drawn width and height have the
// parity of the box's, otherwise a pixel is left over (the icon is half a
// pixel lower or further right). a pixel smaller or larger isn't visible, an
// uneven side is.
float iconSize(unsigned int c, float w, float h, float want)
{
    static std::map<std::tuple<unsigned int, int, int, int>, float> chosen;
    int bw = (int)std::lround(w), bh = (int)std::lround(h), base = (int)ImFloor(want);
    auto key = std::make_tuple(c, bw, bh, base);
    if (auto it = chosen.find(key); it != chosen.end()) return it->second;

    float best = (float)base;
    int best_misses = 4;
    for (int d : {0, -1, 1, -2, 2}) {
        auto g = ImGui::GetFont()->GetFontBaked((float)(base + d))->FindGlyphNoFallback((ImWchar)c);
        if (!g) continue;
        auto ink = inkBox(*g);
        int iw = (int)std::lround(ink.GetWidth()), ih = (int)std::lround(ink.GetHeight());
        // left and right matter more, some icons are odd one way and even
        // the other at every size
        int misses = 2 * ((bw - iw) & 1) + ((bh - ih) & 1);
        if (misses < best_misses) {
            best_misses = misses;
            best = (float)(base + d);
        }
        if (misses == 0) break;
    }
    chosen[key] = best;
    return best;
}

}

ToolbarSize getToolbarSize()
{
    return g_size;
}

void setToolbarSize(ToolbarSize s)
{
    g_size = s;
}

float toolbarButtonSize()
{
    float px = g_size == ToolbarSize::large ? 48.0f : g_size == ToolbarSize::medium ? 38.0f : 30.0f;
    return std::max(px, ImGui::GetFrameHeight());
}

float toolbarGap()
{
    return 3.0f;
}

void toolbarSizeMenu()
{
    using namespace ImGui;
    const char* names[] = {"Small icons", "Medium icons", "Large icons"};
    for (int k = 0; k < 3; k++)
        if (MenuItem(names[k], nullptr, g_size == (ToolbarSize)k)) g_size = (ToolbarSize)k;
}

bool iconButtonAt(const char* icon, const char* tip, float h, bool active, bool enabled)
{
    using namespace ImGui;
    if (active) PushStyleColor(ImGuiCol_Button, GetStyleColorVec4(ImGuiCol_ButtonActive));
    BeginDisabled(!enabled);
    // padding that puts text on the same line in the middle of the button
    PushStyleVar(ImGuiStyleVar_FramePadding, {GetStyle().FramePadding.x, (h - GetFontSize()) / 2});
    PushID(icon);
    bool clicked = Button("##icon", {h, h});
    PopStyleVar();
    // drawn by hand: a label would be centered by its advance and line
    // height, which leaves icons a pixel or two off
    auto min = GetItemRectMin(), max = GetItemRectMax();
    unsigned int c = 0;
    ImTextCharFromUtf8(&c, icon, nullptr);
    float size = iconSize(c, max.x - min.x, max.y - min.y, 0.6f * h);
    if (auto g = GetFont()->GetFontBaked(size)->FindGlyphNoFallback((ImWchar)c)) {
        auto ink = inkBox(*g);
        ImVec2 at{ImFloor((min.x + max.x - ink.Min.x - ink.Max.x) / 2 + 0.5f), ImFloor((min.y + max.y - ink.Min.y - ink.Max.y) / 2 + 0.5f)};
        GetWindowDrawList()->AddText(GetFont(), size, at, GetColorU32(ImGuiCol_Text), icon);
    }
    EndDisabled();
    if (active) PopStyleColor();
    SetItemTooltip("%s", tip);
    PopID();
    return clicked;
}

bool iconButton(const char* icon, const char* tip, bool active, bool enabled)
{
    using namespace ImGui;
    bool clicked = iconButtonAt(icon, tip, toolbarButtonSize(), active, enabled);
    PushID(icon);
    if (BeginPopupContextItem("size")) {
        toolbarSizeMenu();
        EndPopup();
    }
    PopID();
    return clicked;
}

bool smallIconButton(const char* icon, const char* tip, float size)
{
    return iconButtonAt(icon, tip, size > 0 ? size : ImGui::GetFrameHeight(), false, true);
}

void toolbarCenter()
{
    using namespace ImGui;
    SetCursorPosY(GetCursorPosY() + std::max(0.0f, (toolbarButtonSize() - GetFrameHeight()) / 2));
}

bool endsBusy(std::string_view text)
{
    std::string_view room = Icon::busy;
    return text.size() > room.size() && text.substr(text.size() - room.size()) == room;
}

void drawBusy(ImDrawList* list, std::string_view text, float x, const ImRect& box)
{
    using namespace ImGui;
    if (!list || !endsBusy(text)) return;
    std::string_view room = Icon::busy;
    auto start = text.data(), end = text.data() + text.size();
    float before = CalcTextSize(start, end - room.size()).x, width = CalcTextSize(end - room.size(), end).x;
    // a ring a little smaller than the letters, at the right of the room so
    // the rest is the gap after the text. its middle is the box's, which is
    // the middle of the line: the text of a title bar or tab is centered in it.
    float size = std::min(GetFontSize() * 0.75f, width);
    float thick = std::max(1.5f, ImFloor(size * 0.15f));
    ImVec2 center{x + before + width - size / 2, (box.Min.y + box.Max.y) / 2};
    if (center.x - size / 2 >= box.Max.x) return;
    // three quarters of it, a turn a second
    float turn = float(std::fmod(GetTime(), 1.0)) * 2 * IM_PI;
    // not within the window's clip rect, which leaves out its title bar
    list->PushClipRect(box.Min, box.Max, false);
    list->PathArcTo(center, (size - thick) / 2, turn, turn + 1.5f * IM_PI, 24);
    list->PathStroke(GetColorU32(ImGuiCol_Text), ImDrawFlags_None, thick);
    list->PopClipRect();
}

}

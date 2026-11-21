#include <Hd/Zoom.h>
#include <Hd/Icons.h>

#include <imgui_internal.h>

#include <algorithm>
#include <vector>

namespace Hd::Zoom {

namespace {

std::string zoomed;
std::string focused;
// the open windows of this frame, in order
std::vector<std::string> windows;
// per begin: whether ImGui::Begin was called
std::vector<bool> begun;
// select the zoomed tab in the bar once after a change
bool selectTab = false;

// "Terminal: uart###terminal1" is shown as "Terminal: uart"
std::string labelOf(const std::string& name)
{
    auto at = name.find("###");
    return at == std::string::npos ? name : name.substr(0, at);
}

// a window is the same while its id is: what follows "###", so the shown
// part may change (e.g. progress in the title)
std::string_view keyOf(std::string_view name)
{
    auto at = name.find("###");
    return at == std::string_view::npos ? name : name.substr(at);
}

bool same(std::string_view a, std::string_view b)
{
    return keyOf(a) == keyOf(b);
}

// the turning circle of a title ending with Icon::busy, right after Begin:
// on the window's title bar or dock tab, the last item then. hidden tabs get
// it too, Begin returning false. titles are on the left (WindowTitleAlign).
void drawTitleBusy(const std::string& name)
{
    using namespace ImGui;
    auto label = labelOf(name);
    if (!endsBusy(label)) return;
    // not GetCurrentWindow: a window written to doesn't hover its title bar
    auto window = GetCurrentWindowRead();
    auto& style = GetStyle();
    ImRect box = GImGui->LastItemData.Rect;
    if (window->DockIsActive) {
        auto node = window->DockNode;
        if (!node || !node->HostWindow || node->IsHiddenTabBar() || node->IsNoTabBar()) return;
        drawBusy(node->HostWindow->DrawList, label, box.Min.x + style.FramePadding.x, box);
        return;
    }
    if (window->Flags & ImGuiWindowFlags_NoTitleBar) return;
    float pad = style.FramePadding.x;
    if (!(window->Flags & ImGuiWindowFlags_NoCollapse) && style.WindowMenuButtonPosition == ImGuiDir_Left)
        pad += GetFontSize() + 2 * style.ItemInnerSpacing.x;
    drawBusy(window->DrawList, label, box.Min.x + pad, box);
}

}

float barHeight()
{
    auto& style = ImGui::GetStyle();
    return ImGui::GetFrameHeight() + style.WindowPadding.y * 2;
}

bool active()
{
    return !zoomed.empty();
}

const std::string& current()
{
    return zoomed;
}

void set(const std::string& name)
{
    if (!zoomed.empty() && same(zoomed, name)) {
        // the same window under its current title
        zoomed = name;
        return;
    }
    zoomed = name;
    selectTab = true;
}

void clear()
{
    zoomed.clear();
}

void toggle(const std::string& fallback)
{
    if (active())
        clear();
    else if (!focused.empty() || !fallback.empty())
        set(focused.empty() ? fallback : focused);
}

bool begin(const char* name, bool* open, ImGuiWindowFlags flags)
{
    using namespace ImGui;
    if (!open || *open) windows.push_back(name);

    if (active() && !same(zoomed, name)) {
        // its size, position or dock asked for don't go to the next window
        GImGui->NextWindowData.ClearFlags();
        begun.push_back(false);
        return false;
    }

    if (!active()) {
        bool visible = Begin(name, open, flags);
        begun.push_back(true);
        drawTitleBusy(name);
        // the last item is the dock tab or the title bar
        if (IsItemHovered() && IsMouseDoubleClicked(ImGuiMouseButton_Left)) set(name);
        if (BeginPopupContextItem("##zoom")) {
            if (MenuItem("Zoom")) set(name);
            SetItemTooltip("Fill the app with this window, the others become tabs at the bottom");
            EndPopup();
        }
        if (IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) focused = name;
        return visible;
    }

    // the zoomed window over everything docked, above the tab bar. a dock
    // asked for its normal window doesn't apply here.
    GImGui->NextWindowData.HasFlags &= ~ImGuiNextWindowDataFlags_HasDock;
    auto vp = GetMainViewport();
    SetNextWindowPos(vp->WorkPos);
    SetNextWindowSize({vp->WorkSize.x, std::max(1.0f, vp->WorkSize.y - barHeight())});
    // imgui hashes what follows the last ###, so the name's own ### mustn't
    // be in the id or this would be the window of the layout
    std::string key(keyOf(name));
    std::replace(key.begin(), key.end(), '#', '_');
    // the title shown now
    zoomed = name;
    auto id = labelOf(name) + "###zoom_" + key;
    bool visible = Begin(id.c_str(), open, flags | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
    begun.push_back(true);
    drawTitleBusy(name);
    if (IsItemHovered() && IsMouseDoubleClicked(ImGuiMouseButton_Left)) clear();
    if (BeginPopupContextItem("##zoom")) {
        if (MenuItem("Unzoom")) clear();
        EndPopup();
    }
    focused = name;
    return visible;
}

bool shown()
{
    return !begun.empty() && begun.back();
}

void end()
{
    if (begun.empty()) return;
    bool was = begun.back();
    begun.pop_back();
    if (was) ImGui::End();
}

void endFrame()
{
    using namespace ImGui;
    auto list = std::move(windows);
    windows.clear();
    if (active() && std::none_of(list.begin(), list.end(), [](auto& n) { return same(n, zoomed); })) clear();
    if (!active()) return;

    auto vp = GetMainViewport();
    float h = barHeight();
    SetNextWindowPos({vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - h});
    SetNextWindowSize({vp->WorkSize.x, h});
    Begin("##zoom_bar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
    float button = GetFrameHeight();
    std::string chosen;
    // the tabs, with room for the unzoom button at the right
    float room = std::max(1.0f, GetContentRegionAvail().x - button - GetStyle().ItemSpacing.x);
    BeginChild("tabs", {room, GetFrameHeight()}, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    if (BeginTabBar("windows", ImGuiTabBarFlags_FittingPolicyScroll)) {
        for (auto& name : list) {
            PushID(std::string(keyOf(name)).c_str());
            ImGuiTabItemFlags flags = selectTab && same(name, zoomed) ? ImGuiTabItemFlags_SetSelected : 0;
            auto label = labelOf(name);
            bool selected = BeginTabItem((label + "###tab").c_str(), nullptr, flags);
            ImRect tab = GImGui->LastItemData.Rect;
            drawBusy(GetWindowDrawList(), label, tab.Min.x + GetStyle().FramePadding.x, tab);
            if (selected) {
                if (!selectTab && !same(name, zoomed)) chosen = name;
                EndTabItem();
            }
            PopID();
        }
        EndTabBar();
    }
    EndChild();
    selectTab = false;
    SameLine();
    if (smallIconButton(Icon::unzoom, "Unzoom: back to the layout")) clear();
    End();
    if (!chosen.empty()) set(chosen);
}

}

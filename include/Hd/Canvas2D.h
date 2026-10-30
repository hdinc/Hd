#pragma once

#include <Hd/FrameBuffer.h>
#include <Hd/Texture.h>

#include <glm/glm.hpp>
#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace Hd {

// 2d view of a world drawn into a framebuffer in an imgui window: zoom
// with the mouse wheel (smoothly) or a middle drag, pan with a right drag
class Canvas2D {
public:
    // every frame before the input and drawing: the size of the viewport in
    // pixels. moves a zoom on.
    void beginFrame(glm::vec2 size);

    // mouse position in normalized device coordinates [-1, 1], y up
    void setMouse(glm::vec2 ndc) { mouse = ndc; }

    // zoom and pan from imgui input. call only while the canvas is hovered
    void handleViewInput();

    const glm::mat4& vp() const { return m_vp; }
    float getZoom() const { return zoom; }

    glm::vec2 toWorld(glm::vec2 ndc) const;
    glm::vec2 mouseWorld() const { return toWorld(mouse); }

    // converts a distance in pixels to world units, for hit test tolerances
    float pixels(float px) const;

    // the same zoom as another view of the same world, and the same point
    // in its bottom left corner: views taking turns in the same window share
    // that corner even when one has a toolbar row more above. takes effect
    // with the next beginFrame.
    void copyCamera(const Canvas2D& other)
    {
        zoom = targetZoom = other.zoom;
        center = other.center;
        anchor = other.toWorld({-1.0f, -1.0f});
        anchored = true;
        recalc();
    }

private:
    void recalc();

    glm::mat4 m_vp{1.0f};
    glm::vec2 size{1.0f, 1.0f};
    glm::vec2 mouse{0.0f, 0.0f};

    glm::vec2 center{0.0f, 0.0f};
    float zoom = 0.01f;

    // the wheel sets the zoom to go to, beginFrame moves there over a few
    // frames with this world point staying at this screen point
    float targetZoom = 0.01f;
    glm::vec2 zoomWorld{0.0f, 0.0f};
    glm::vec2 zoomNdc{0.0f, 0.0f};

    // keep this world point in the bottom left corner, see copyCamera
    glm::vec2 anchor{0.0f, 0.0f};
    bool anchored = false;

    bool panning = false;
    glm::vec2 panLast{0.0f, 0.0f};

    bool dragZooming = false;
    float zoomLastY = 0.0f;
};

// a canvas's size in the remaining space of the current imgui window, in
// whole imgui units, and in framebuffer pixels: imgui units times the
// display's scale (more than 1 on hi-dpi screens). the framebuffer is drawn
// at the screen's resolution and shown 1:1, so it isn't stretched and small
// text stays sharp. both are 0 if there's no room or tex is too small.
struct CanvasSize {
    ImVec2 size;
    ImVec2 pixels;
};

inline CanvasSize canvasRoom(Hd::Texture& tex)
{
    float scale = ImGui::GetIO().DisplayFramebufferScale.x;
    auto size = ImGui::GetContentRegionAvail();
    size.x = std::min(std::floor(size.x), std::floor(tex.GetWidth() / scale));
    size.y = std::min(std::floor(size.y), std::floor(tex.GetHeight() / scale));
    if (size.x < 1.0f || size.y < 1.0f) return {};
    return {size, {std::round(size.x * scale), std::round(size.y * scale)}};
}

// a canvas in the remaining space of the current imgui window. input(hovered)
// runs after the view got its size and mouse, hovered when the mouse is on
// it and the window has the focus. draw() then draws into fb, see
// canvasRoom. min/max get the screen rectangle. false if there's no room.
template <class Input, class Draw>
bool canvasWidget(Canvas2D& c, Hd::Texture& tex, Hd::FrameBuffer& fb, Input input, Draw draw, ImVec2* min = nullptr, ImVec2* max = nullptr)
{
    using namespace ImGui;

    auto [size, px] = canvasRoom(tex);
    if (size.x < 1.0f) return false;

    auto screenPos = GetCursorScreenPos();
    auto cursor = GetCursorPos();
    InvisibleButton("canvas", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool hovered = IsItemHovered() && IsWindowFocused();

    auto mouse = GetMousePos();
    c.beginFrame({size.x, size.y});
    c.setMouse({2.0f * (mouse.x - screenPos.x) / size.x - 1.0f, 1.0f - 2.0f * (mouse.y - screenPos.y) / size.y});
    input(hovered);

    fb.Bind((int)px.x, (int)px.y);
    draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // the texture is bigger than the view, only show the drawn corner
    SetCursorPos(cursor);
    Image((ImTextureID)tex.Id(), size, {0, px.y / tex.GetHeight()}, {px.x / tex.GetWidth(), 0});
    if (min) *min = screenPos;
    if (max) *max = {screenPos.x + size.x, screenPos.y + size.y};
    return true;
}

}

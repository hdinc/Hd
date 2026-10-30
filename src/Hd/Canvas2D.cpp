#include <Hd/Canvas2D.h>

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <cmath>

namespace Hd {

void Canvas2D::beginFrame(glm::vec2 s)
{
    size = glm::max(s, glm::vec2{1.0f, 1.0f});
    recalc();

    if (targetZoom != zoom) {
        // most of the way in about 0.15 s, evenly in the logarithm
        float t = 1.0f - std::exp(-ImGui::GetIO().DeltaTime * 20.0f);
        float k = std::log(targetZoom / zoom);
        zoom = std::abs(k) < 0.002f ? targetZoom : zoom * std::exp(k * t);
        recalc();
        center += zoomWorld - toWorld(zoomNdc);
        recalc();
    }
    if (anchored) {
        anchored = false;
        center += anchor - toWorld({-1.0f, -1.0f});
        recalc();
    }
}

void Canvas2D::handleViewInput()
{
    auto& io = ImGui::GetIO();

    if (io.MouseWheel != 0) {
        // the point under the cursor stays there while zooming
        targetZoom = glm::clamp(targetZoom * std::pow(1.2f, io.MouseWheel), 1.0f / 10000.0f, 1.0f);
        zoomNdc = mouse;
        zoomWorld = mouseWorld();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        panning = true;
        panLast = mouse;
    }
    if (panning) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            center += toWorld(panLast) - mouseWorld();
            recalc();
            panLast = mouse;
            // a zoom going on keeps the point under the cursor
            zoomWorld = toWorld(zoomNdc);
        } else {
            panning = false;
        }
    }

    // middle drag: up zooms in, down out, about the center of the view
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
        dragZooming = true;
        zoomLastY = mouse.y;
        zoomNdc = glm::vec2{0.0f, 0.0f};
        zoomWorld = toWorld(zoomNdc);
    }
    if (dragZooming) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            float dy = mouse.y - zoomLastY;
            zoomLastY = mouse.y;
            if (dy != 0.0f) {
                // the height of the view is a factor of about 20
                zoom = targetZoom = glm::clamp(zoom * std::exp(dy * 1.5f), 1.0f / 10000.0f, 1.0f);
                recalc();
                center += zoomWorld - toWorld(zoomNdc);
                recalc();
            }
        } else {
            dragZooming = false;
        }
    }
}

glm::vec2 Canvas2D::toWorld(glm::vec2 ndc) const
{
    auto r = glm::inverse(m_vp) * glm::vec4(ndc, 0.0f, 1.0f);
    return {r.x, r.y};
}

float Canvas2D::pixels(float px) const
{
    return px * 2.0f / size.x / m_vp[0][0];
}

void Canvas2D::recalc()
{
    // zoom is the scale of the shorter side, so a world unit is square
    glm::vec3 s(zoom, zoom, 1.0f);
    if (size.x > size.y)
        s.y *= size.x / size.y;
    else
        s.x *= size.y / size.x;

    m_vp = glm::translate(glm::scale(glm::mat4(1.0f), s), {-center, 0.0f});
}

}

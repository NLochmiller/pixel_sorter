#ifndef IMGUI_IMAGEZOOMABLE_HPP_
#define IMGUI_IMAGEZOOMABLE_HPP_

#include "imgui.h"
namespace ImGui {

// Must provide the texture id, and width height of
// Can optionally provide the tint color (default none), and the border color
// (default: ImGuiCol_Border)
// returns success
bool ImageZoomable(
    ImTextureID textureId, ImVec2 textureSize, float previewSize = 32.0f,
    float zoomFactor = 4.0f, ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
    ImVec4 borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border));

bool ImageZoomable(
    ImTextureID textureId, ImVec2 textureSize, ImVec2 displaySize,
    float previewSize = 32.0f, float zoomFactor = 4.0f,
    ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
    ImVec4 borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border));

}; // namespace ImGui

#endif

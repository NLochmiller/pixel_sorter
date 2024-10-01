#ifndef IMGUI_IMAGEZOOMABLE_HPP_
#define IMGUI_IMAGEZOOMABLE_HPP_

#include "imgui.h"
namespace ImGui {

// Must provide the texture id, and width height of
// Can optionally provide the tint color (default none), and the border color
// (default: ImGuiCol_Border)
// previewNum is the number of pixels that are taken from the texture and shown
// in the preview
// previewSize is the onscreen size of the preview square
// returns success
bool ImageZoomable(
    ImTextureID textureId, ImVec2 textureSize, float previewNum = 32.0f,
    float previewSize = 100.0f,
    ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
    ImVec4 borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border));

bool ImageZoomable(
    ImTextureID textureId, ImVec2 textureSize, ImVec2 displaySize,
    float previewNum = 32.0f, float previewSize = 100.0f,
    ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
    ImVec4 borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border));

}; // namespace ImGui

#endif

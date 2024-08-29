#include "ImGui_ImageZoomable.hpp"
#include <cstdio>

// Must provide the texture id, and width height of
// Can optionally provide the tint color (default none), and the border color
// (default: ImGuiCol_Border)
// Returns success
bool ImGui::ImageZoomable(ImTextureID textureId, float textureWidth,
                          float textureHeight, float previewSize, float zoom,
                          ImVec4 tintColor, ImVec4 borderColor) {
  if (textureId == NULL) {
    fprintf(stderr, "ImageZoomable: Nonexistent texture!\n");
    return false;
  }

  if (textureWidth <= 0) {
    fprintf(stderr, "ImageZoomable: Invalide width %f\n", textureWidth);
    return false;
  }
  if (textureHeight <= 0) {
    fprintf(stderr, "ImageZoomable: Invalide height %f\n", textureHeight);
    return false;
  }
  if (previewSize <= 0) {
    fprintf(stderr, "ImageZoomable: Invalide preview size %f\n", previewSize);
    return false;
  }
  if (zoom <= 0) {
    fprintf(stderr, "ImageZoomable: Invalide zoom %f\n", zoom);
    return false;
  }

  // TODO:  Do error checking here
  ImGuiIO &io = ImGui::GetIO();
  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right

  ImGui::Image(textureId, ImVec2(textureWidth, textureHeight), uv_min, uv_max,
               tintColor, borderColor);
  if (ImGui::BeginItemTooltip()) {
    float previewX = io.MousePos.x - pos.x - previewSize * 0.5f;
    float previewY = io.MousePos.y - pos.y - previewSize * 0.5f;

    if (previewX < 0.0f) {
      previewX = 0.0f; // Clamp to 0
    } else if (previewX > textureWidth - previewSize) {
      previewX = textureWidth - previewSize; // Clamp to right of image
    }
    if (previewY < 0.0f) {
      previewY = 0.0f; // Clamp to top of image
    } else if (previewY > textureHeight - previewSize) {
      previewY = textureHeight - previewSize; // Clamp to bottom of image
    }
    ImGui::Text("Min: (%.2f, %.2f)", previewX, previewY);
    ImGui::Text("Max: (%.2f, %.2f)", previewX + previewSize,
                previewY + previewSize);
    ImVec2 uv0 = ImVec2((previewX) / textureWidth, (previewY) / textureHeight);
    ImVec2 uv1 = ImVec2((previewX + previewSize) / textureWidth,
                        (previewY + previewSize) / textureHeight);
    ImGui::Image(textureId, ImVec2(previewSize * zoom, previewSize * zoom), uv0,
                 uv1, tintColor, borderColor);
    ImGui::EndTooltip();
  }
  return true;
}

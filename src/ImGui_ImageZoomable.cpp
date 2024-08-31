#include "ImGui_ImageZoomable.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

// Must provide the texture id, and width height of
// Can optionally provide the tint color (default none), and the border color
// (default: ImGuiCol_Border)
// previewNum is the number of pixels that are taken from the texture and shown
// in the preview
// previewSize is the onscreen size of the preview square
// Returns success
bool ImGui::ImageZoomable(ImTextureID textureId, ImVec2 textureSize,
                          float previewNum, float previewSize, ImVec4 tintColor,
                          ImVec4 borderColor) {
  if (textureId == NULL) {
    fprintf(stderr, "ImageZoomable: Nonexistent texture!\n");
    return false;
  }
  if (previewNum <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid preview size %f\n", previewNum);
    return false;
  }
  if (previewSize <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid zoom %f\n", previewSize);
    return false;
  }

  float textureWidth = textureSize.x;
  float textureHeight = textureSize.y;
  if (textureWidth <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid width %f\n", textureWidth);
    return false;
  }
  if (textureHeight <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid height %f\n", textureHeight);
    return false;
  }

  ImGuiIO &io = ImGui::GetIO();
  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right
  // The amount that preview is multipled by to be the desired size
  float zoom = previewSize / previewNum;
  ImGui::Image(textureId, ImVec2(textureWidth, textureHeight), uv_min, uv_max,
               tintColor, borderColor);
  if (ImGui::BeginItemTooltip()) {
    float displayX = io.MousePos.x - pos.x - previewNum * 0.5f;
    float displayY = io.MousePos.y - pos.y - previewNum * 0.5f;
    displayX = std::clamp(displayX, 0.0f, textureWidth - previewNum);
    displayY = std::clamp(displayY, 0.0f, textureHeight - previewNum);

    ImGui::Text("X: %d", (int)displayX + (int)previewNum / 2);
    ImGui::Text("Y: %d", (int)displayY + (int)previewNum / 2);
    ImVec2 uv0 = ImVec2((displayX) / textureWidth, (displayY) / textureHeight);
    ImVec2 uv1 = ImVec2((displayX + previewNum) / textureWidth,
                        (displayY + previewNum) / textureHeight);
    ImGui::Image(textureId,
                 ImVec2(floor(previewNum * zoom), floor(previewNum * zoom)),
                 uv0, uv1, tintColor, borderColor);
    ImGui::EndTooltip();
  }
  return true;
}

// returns the point relative to srcRect to be relative to dstRect
ImVec2 scalePoint(ImVec2 srcRect, ImVec2 dstRect, ImVec2 pt) {
  // Compute the corresponding position in the destination rectangle
  ImVec2 scaledPoint;
  scaledPoint.x = pt.x * dstRect.x / srcRect.x;
  scaledPoint.y = pt.y * dstRect.y / srcRect.y;

  return scaledPoint;
}

// Must provide the texture id, and width height of source texture.
// Must also provide desired displaySize of image, being the size on screen the
// image the user can hover over will be.
// Can optionally provide the tint color (default none),
// and the border color (default: ImGuiCol_Border). Returns success
bool ImGui::ImageZoomable(ImTextureID textureId, ImVec2 textureSize,
                          ImVec2 displaySize, float previewNum,
                          float previewSize, ImVec4 tintColor,
                          ImVec4 borderColor) {
  if (textureId == NULL) {
    fprintf(stderr, "ImageZoomable: Nonexistent texture!\n");
    return false;
  }
  if (previewNum <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid preview size %f\n", previewNum);
    return false;
  }
  if (previewSize <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid zoom %f\n", previewSize);
    return false;
  }

  float textureWidth = textureSize.x;
  float textureHeight = textureSize.y;
  if (textureWidth <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid width %f\n", textureWidth);
    return false;
  }
  if (textureHeight <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid height %f\n", textureHeight);
    return false;
  }

  float displayWidth = displaySize.x;
  float displayHeight = displaySize.y;
  if (displayWidth <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid width %f\n", displayWidth);
    return false;
  }
  if (displayHeight <= 0) {
    fprintf(stderr, "ImageZoomable: Invalid height %f\n", displayHeight);
    return false;
  }

  ImGuiIO &io = ImGui::GetIO();
  ImVec2 pos = ImGui::GetCursorScreenPos();
  // The amount that preview is multipled by to be the desired size
  float zoom = previewSize / previewNum;
  ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right

  // Draw normal image
  ImGui::Image(textureId, ImVec2(displayWidth, displayHeight), uv_min, uv_max,
               tintColor, borderColor);

  if (ImGui::BeginItemTooltip()) {
    float displayX = io.MousePos.x - pos.x;
    float displayY = io.MousePos.y - pos.y;
    // Scale the cursors location from the display to the texture
    ImVec2 textureOffset =
        scalePoint(displaySize, textureSize, ImVec2(displayX, displayY));
    // Center preview around cursor
    textureOffset.x -= (previewNum * 0.5f);
    textureOffset.y -= (previewNum * 0.5f);
    // Clamp the preview to inside the image, preventing clipping
    textureOffset.x =
        std::clamp(textureOffset.x, 0.0f, textureWidth - previewNum);
    textureOffset.y =
        std::clamp(textureOffset.y, 0.0f, textureHeight - previewNum);

    ImVec2 uv0 =
        ImVec2(textureOffset.x / textureWidth, textureOffset.y / textureHeight);
    ImVec2 uv1 = ImVec2((textureOffset.x + previewNum) / textureWidth,
                        (textureOffset.y + previewNum) / textureHeight);
    ImGui::Image(textureId,
                 ImVec2(floor(previewNum * zoom), floor(previewNum * zoom)),
                 uv0, uv1, tintColor, borderColor);
    ImGui::EndTooltip();
  }
  return true;
}

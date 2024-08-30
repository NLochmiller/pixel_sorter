#include "ImGui_ImageZoomable.hpp"
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
    float previewX = io.MousePos.x - pos.x - previewNum * 0.5f;
    float previewY = io.MousePos.y - pos.y - previewNum * 0.5f;

    if (previewX < 0.0f) {
      previewX = 0.0f; // Clamp to 0
    } else if (previewX > textureWidth - previewNum) {
      previewX = textureWidth - previewNum; // Clamp to right of image
    }
    if (previewY < 0.0f) {
      previewY = 0.0f; // Clamp to top of image
    } else if (previewY > textureHeight - previewNum) {
      previewY = textureHeight - previewNum; // Clamp to bottom of image
    }
    ImGui::Text("X: %d", (int)previewX + (int)previewNum / 2);
    ImGui::Text("Y: %d", (int)previewY + (int)previewNum / 2);
    ImVec2 uv0 = ImVec2((previewX) / textureWidth, (previewY) / textureHeight);
    ImVec2 uv1 = ImVec2((previewX + previewNum) / textureWidth,
                        (previewY + previewNum) / textureHeight);
    ImGui::Image(textureId,
                 ImVec2(floor(previewNum * zoom), floor(previewNum * zoom)),
                 uv0, uv1, tintColor, borderColor);
    ImGui::EndTooltip();
  }
  return true;
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
  ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right
  // The amount that preview is multipled by to be the desired size
  float zoom = previewNum / previewSize;
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
    ImGui::Text("X: %d", (int)previewX + (int)previewSize / 2);
    ImGui::Text("Y: %d", (int)previewY + (int)previewSize / 2);
    ImVec2 uv0 = ImVec2((previewX) / textureWidth, (previewY) / textureHeight);
    ImVec2 uv1 = ImVec2((previewX + previewSize) / textureWidth,
                        (previewY + previewSize) / textureHeight);
    ImGui::Image(textureId,
                 ImVec2(floor(previewSize * zoom), floor(previewSize * zoom)),
                 uv0, uv1, tintColor, borderColor);
    ImGui::EndTooltip();
  }
  return true;
  // ImGui::Image(textureId, ImVec2(displayWidth, displayHeight), uv_min,
  // uv_max,
  //              tintColor, borderColor);
  // if (ImGui::BeginItemTooltip()) {
  //   float previewX = io.MousePos.x - pos.x - previewSize * 0.5f;
  //   float previewY = io.MousePos.y - pos.y - previewSize * 0.5f;

  //   if (previewX < 0.0f) {
  //     previewX = 0.0f; // Clamp to 0
  //   } else if (previewX > textureWidth - previewSize) {
  //     previewX = textureWidth - previewSize; // Clamp to right of image
  //   }
  //   if (previewY < 0.0f) {
  //     previewY = 0.0f; // Clamp to top of image
  //   } else if (previewY > textureHeight - previewSize) {
  //     previewY = textureHeight - previewSize; // Clamp to bottom of image
  //   }

  //   // Scale preview size to full texture size
  //   // previewX *= textureWidth / displayWidth;
  //   // previewY *= textureHeight / displayHeight;

  //   ImGui::Text("X: %d", (int)previewX + (int)previewSize / 2);
  //   ImGui::Text("Y: %d", (int)previewY + (int)previewSize / 2);
  //   ImVec2 uv0 = ImVec2((previewX) / textureWidth, (previewY) /
  //   textureHeight); ImVec2 uv1 = ImVec2((previewX + previewSize) /
  //   textureWidth,
  //                       (previewY + previewSize) / textureHeight);
  //   ImGui::Image(textureId, ImVec2(previewSize * zoom, previewSize * zoom),
  //   uv0,
  //                uv1, tintColor, borderColor);
  //   ImGui::EndTooltip();
  // }
  // return true;
}

#include "Knob.hpp"
#include "imgui.h"
#include <cmath>

// Finds the angle from src to dest
float findAngleDifference(ImVec2 src, ImVec2 dest) {
  return (float)atan2(dest.y - src.y, dest.x - src.x);
}

//

bool ImGui::Knob(const char *label, float *angle, float radius) {
  bool has_value_changed = false;

  // Get io and styling
  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle &style = ImGui::GetStyle();
  // If ever want text, use float line_height = ImGui::GetTextLineHeight();

  // Get positioning data
  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  // The area occupied by this knob
  ImVec2 area = ImVec2(radius * 2, radius * 2 + style.ItemInnerSpacing.y);

  // Use an invisible button to detect user input
  ImGui::InvisibleButton(label, area);
  bool is_active = ImGui::IsItemActive();
  bool is_hovered = ImGui::IsItemHovered();
  // If the user has moved at all, while clicking on this
  if (is_active && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)) {
    ImVec2 mouse_pos = io.MousePos;
    *angle = findAngleDifference(center, mouse_pos);
    // Adjust to always be positive
    if (*angle < 0) {
      *angle = 2 * M_PI + *angle;
    }
    has_value_changed = true;
  }

  // Precalculate values needed
  float angle_cos = cosf(*angle), angle_sin = sinf(*angle);

  /* Draw knob base */
  // Set base color based on if the knob is hovered
  ImU32 base_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
  if (is_hovered) {
    base_color = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
  }
  
  draw_list->AddCircleFilled(center, radius, base_color, 0);

  /* Draw indicator */
  int internal_radius = std::max(1, (int)radius / 5);
  int indicator_thickness = std::max(2, (int)radius / 25);
  int indicator_padding = std::max(1, (int)radius / 6);

  ImU32 indicator_color = ImGui::GetColorU32(ImGuiCol_Text);
  ImVec2 p1 = ImVec2(center.x + angle_cos * internal_radius,
                     center.y + angle_sin * internal_radius);

  ImVec2 p2 = ImVec2(center.x + angle_cos * (radius - indicator_padding),
                     center.y + angle_sin * (radius - indicator_padding));
  draw_list->AddLine(p1, p2, indicator_color, indicator_thickness);
  return has_value_changed;
}

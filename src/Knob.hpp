/*
 * A simple knob widget for DearImGui. Angles are in radians
 */

#ifndef KNOB_HPP_
#define KNOB_HPP_

namespace ImGui {
  // Simple knob. Works in radians. 
  bool Knob(const char *label, double &angle, float radius);
}

#endif // KNOB_HPP_

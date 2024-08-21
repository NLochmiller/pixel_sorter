// All functions in this file work in percentages, from 0.0 to 1.0
// Convert colors from RGB to a single value

#ifndef COLORCONVERSION_HPP_
#define COLORCONVERSION_HPP_

#define COLORCONVERTER_ARGS double r, double g, double b

typedef double ColorConverter(COLORCONVERTER_ARGS);

namespace ColorConversion {
// Just red
ColorConverter red;

// Just green
ColorConverter green;

// Just blue
ColorConverter blue;

// The average of r,g,b
ColorConverter average;

// Return the minimum of (R,G,B)
ColorConverter minimum;

// Return the maximum of (R,G,B)
ColorConverter maximum;

// The range aka chroma
ColorConverter chroma;

// Hue in HSV and HSL
ColorConverter hue;

// Saturation (HSV)
ColorConverter saturation;

// Value (HSV)
ColorConverter value;

// Saturation (HSL)
ColorConverter saturation_HSL;

// Lightness (HSL)
ColorConverter lightness;

} // namespace ColorConversion
#endif // COLORCONVERSION_HPP

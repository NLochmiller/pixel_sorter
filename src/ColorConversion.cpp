#include "ColorConversion.hpp"
#include <algorithm>
#include <cmath>



// Get the max of RGB
double maxRGB(double r, double g, double b) {
  return std::max(r, std::max(g, b));
}

// Get the min of RGB
double minRGB(double r, double g, double b) {
  return std::min(r, std::min(g, b));
}

// All ColorConversion functions in this file work in percentages, from 0.0
// to 1.0

// Just red

double ColorConversion::red(double r, double g, double b) { return r; }

// Just green
double ColorConversion::green(double r, double g, double b) { return g; }

// Just blue
double ColorConversion::blue(double r, double g, double b) { return b; }

// The average of r, g, b
double ColorConversion::average(double r, double g, double b) {
  return (r + g + b) / 3.0;
}

// Return the value of (R,G,B)
// V = max(R,G,B)
double ColorConversion::value(double r, double g, double b) {
  // Return the max(r,g,b)
  return maxRGB(r, g, b);
}

// Return the maximum of (R,G,B)
double ColorConversion::maximum(double r, double g, double b) {
  return maxRGB(r, g, b);
}

// Return the minimum of (R,G,B)
double ColorConversion::minimum(double r, double g, double b) {
  // Return the min(r,g,b)
  return minRGB(r, g, b);
}

// The range aka chroma
double ColorConversion::chroma(double r, double g, double b) {
  // Calculate chroma
  double max = maxRGB(r, g, b);
  double min = minRGB(r, g, b);
  double chroma = max - min;

  return chroma;
}

double ColorConversion::lightness(double r, double g, double b) {
  // Calculate lightness
  double max = maxRGB(r, g, b);
  double min = minRGB(r, g, b);
  double lightness = (max + min) / 2.0;

  return lightness;
}

double ColorConversion::hue(double r, double g, double b) {
  // Calculate chroma
  double max = maxRGB(r, g, b);
  double min = minRGB(r, g, b);
  double chroma = max - min;

  // Calculate hue
  double hue = 0;
  if (chroma == 0) {
    hue = 0;
  } else if (max == r) {
    hue = 60 * fmod((g - b) / chroma, 6.0);
  } else if (max == g) {
    hue = 60 * (((b - r) / chroma) + 2);
  } else if (max == b) {
    hue = 60 * (((r - g) / chroma) + 4);
  }

  // Hue is usually [0, 360], so convert to [0, 1]
  return hue / 360;
}

// Saturation (HSV)
// TODO: BUGGGY FIX
double ColorConversion::saturation(double r, double g, double b) {
  // Calculate value (called max) and chroma
  double max = maxRGB(r, g, b);
  double min = minRGB(r, g, b);
  double chroma = max - min;

  // Calculate saturation
  double saturation = 0;
  // If V = 0
  if (max == 0) {
    saturation = 0;
  } else {
    saturation = chroma / max;
  }

  return saturation;
}

// HSL saturation
// TODO: BUGGY FIX SEE 20x20 gradient on 90d all values, bad in bottom right
double ColorConversion::saturation_HSL(double r, double g, double b) {
  // Calculate Lightness
  double max = maxRGB(r, g, b);
  double min = minRGB(r, g, b);
  double lightness = (max + min) / 2;

  double saturation = 0;
  if (lightness == 0 || lightness == 1) {
    saturation = 0;
  } else {
    // S = (V-L) / min(L, 1-L)
    saturation = (max - lightness) / std::min(lightness, 1 - lightness);
  }

  return saturation;
}

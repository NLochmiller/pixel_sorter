#ifndef COLOR_CONVERSION_H
#define COLOR_CONVERSION_H

#include <SDL2/SDL.h>

// Just red
double ColCon_Red(double r, double g, double b);

// Just green
double ColCon_Green(double r, double g, double b);

// Just blue
double ColCon_Blue(double r, double g, double b);

// The average of r,g,b
double ColCon_Average(double r, double g, double b);

// Return the minimum of (R,G,B)
double ColCon_Minimum(double r, double g, double b);

// The range aka chroma
double ColCon_Chroma(double r, double g, double b);

// Hue in HSV and HSL
double ColCon_Hue(double r, double g, double b);

// Saturation (HSV)
double ColCon_Saturation(double r, double g, double b);

// Value (HSV)
double ColCon_Value(double r, double g, double b);

// Saturation (HSL)
double ColCon_Saturation_HSL(double r, double g, double b);

// Lightness (HSL)
double ColCon_Lightness(double r, double g, double b);

#endif /* COLOR_CONVERSION_H */

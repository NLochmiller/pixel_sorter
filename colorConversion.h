#ifndef COLORCONVERSION_H
#define COLORCONVERSION_H

#include <SDL.h>

// Just red
double calcRed(double r, double g, double b);

// Just green
double calcGreen(double r, double g, double b);

// Just blue
double calcBlue(double r, double g, double b);

// The average of r,g,b
double calcAverage(double r, double g, double b);

// Return the minimum of (R,G,B)
double calcMinimum(double r, double g, double b);

// The range aka chroma
double calcChroma(double r, double g, double b);

// Hue in HSV and HSL
double calcHue(double r, double g, double b);

// Saturation (HSV)
double calcSaturation(double r, double g, double b);

// Value (HSV)
double calcValue(double r, double g, double b);

// Saturation (HSL)
double calcSaturation_HSL(double r, double g, double b);

// Lightness (HSL)
double calcLightness(double r, double g, double b);

#endif

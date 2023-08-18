#include <SDL2/SDL.h>
#include <math.h>
#include "color_conversion.h"



// Convert a value between [0,255] to [0,1]
double convertTo1(Uint8 n){
    return n/255.0;
}

// Convert a value between [0,1] to [0,255]
Uint8 convertFrom1(double n){
    return (n * 255);
}

// Needed specifically for HSV
// Convert a value between [0,360] to [0,255]
Uint8 convertFrom360(double n){
    return n * 255 / 360;
}

// Return the max of two doubles
double maxd(double a, double b){
    if ( a > b ){
        return a;
    } else {
        return b;
    }
}

// Return the min of two doubles
double mind(double a, double b){
    if ( a < b ){
        return a;
    } else {
        return b;
    }
}

// Get the max of RGB
double maxRGB(double r, double g, double b){
    return maxd(r, maxd(g, b));
}

// Get the min of RGB
double minRGB(double r, double g, double b){
    return mind(r, mind(g, b));
}


// All calculation functions in this file must follow the format:
// Uint8 functionName(Uint8 R, Uint8 G, Uint8 B)

// Just red
double calcRed(double r, double g, double b){
    return r;
}

// Just green
double calcGreen(double r, double g, double b){
    return g;
}

// Just blue
double calcBlue(double r, double g, double b){
    return b;
}

// The average of r, g, b
double calcAverage(double r, double g, double b){
    return (r + g + b)/3.0;
}

// Return the value of (R,G,B)
// V = max(R,G,B)
double calcValue(double r, double g, double b){  
    // Return the max(r,g,b)
    return maxRGB(r, g, b);
}

// Return the minimum of (R,G,B)
double calcMinimum(double r, double g, double b){
    // Return the min(r,g,b)
    return minRGB(r, g, b);
}


// The range aka chroma
double calcChroma(double r, double g, double b){
    // Calculate chroma
    double max = maxRGB(r, g, b);
    double min = minRGB(r, g, b);
    double chroma = max - min;

    return chroma;
}

double calcLightness(double r, double g, double b){
    // Calculate lightness
    double max = maxRGB(r, g, b);
    double min = minRGB(r, g, b);
    double lightness = (max + min) / 2.0;

    return lightness;
}

double calcHue(double r, double g, double b){
    // Calculate chroma
    double max = maxRGB(r, g, b);
    double min = minRGB(r, g, b);
    double chroma = max - min;

    // Calculate hue
    double hue = 0;
    if ( chroma == 0 ){
        hue = 0;
    } else if (max == r){
        hue = 60 *  fmod((g - b)/chroma , 6.0 );
    } else if (max == g){
        hue = 60 * ( ( (b - r) / chroma ) + 2 );
    } else if (max == b){
        hue = 60 * ( ( (r - g) / chroma ) + 4 );
    }

    // Hue is usually [0, 360], so convert to [0, 1]
    return hue/360;
}

// Saturation (HSV)
double calcSaturation(double r, double g, double b){
    // Calculate chroma
    double max = maxRGB(r, g, b);
    double min = minRGB(r, g, b);
    double chroma = max - min;

    // Calculate saturation
    double saturation = 0;
    if ( max == 0 ){
        saturation = 0;
    } else {
        saturation = chroma / max;
    }

    return convertFrom1(saturation);
}

// HSL saturation
double calcSaturation_HSL(double r, double g, double b){
    // Calculate Lightness
    double max = maxRGB(r, g, b);
    double min = minRGB(r, g, b);
    double lightness = (max + min)/2;

    double saturation = 0;
    if ( lightness == 0 || lightness == 1 ){
        saturation = 0;
    } else {
        // S = (V-L) / min(L, 1-L)
        saturation = (max-lightness) / mind(lightness, 1-lightness);
    }

    return convertFrom1(saturation);
}

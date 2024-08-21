#ifndef PIXELSORTER_HPP_
#define PIXELSORTER_HPP_

#include "ColorConversion.hpp"
#include "SDL_pixels.h"
#include <cstdint>


typedef std::pair<int, int> point_ints;
// By default, assumes format
typedef uint32_t PixelSorter_Pixel_t;
typedef uint8_t PixelSorter_value_t;
#define PIXELSORTER_VALUE_T_MAX UINT8_MAX
#define PRECISION UINT8_MAX
typedef long Count_t;

namespace PixelSorter {
void sort(PixelSorter_Pixel_t *&inputPixels,
          PixelSorter_Pixel_t *&outputPixels, point_ints *points,
          int numPoints, int width, int height, int startX, int startY,
          int endX, int endY, double valueMin, double valueMax,
          ColorConverter *converter, SDL_PixelFormat *format);
}

#endif // PIXELSORTER_HPP_

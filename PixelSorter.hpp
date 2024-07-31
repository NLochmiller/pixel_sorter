#ifndef PIXELSORTER_HPP_
#define PIXELSORTER_HPP_

#include "SDL_surface.h"
#include <cstdint>
#include <string>

typedef std::pair<int, int> point_ints;
// By default, assumes format
typedef uint32_t PixelSorter_Pixel_t;

namespace PixelSorter {
void sort(PixelSorter_Pixel_t *&input_pixels,
          PixelSorter_Pixel_t *&output_pixels, point_ints *points,
          int numPoints, int width, int height, int startX, int startY,
          int endX, int endY, SDL_Surface *input_test = NULL);
}

#endif // PIXELSORTER_HPP_

#ifndef PIXELSORTER_HPP_
#define PIXELSORTER_HPP_

#include <cstdint>

typedef uint32_t Uint32; // Compadability with SDL2

// By default, assumes format
typedef Uint32 PixelSorter_Pixel_t;

namespace PixelSorter {
void sort(PixelSorter_Pixel_t *&input_pixels,
          PixelSorter_Pixel_t *&output_pixels);
}

#endif // PIXELSORTER_HPP_

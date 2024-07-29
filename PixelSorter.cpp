#include "PixelSorter.hpp"
#include <cstdio>

void PixelSorter::sort(PixelSorter_Pixel_t *&input_pixels,
                       PixelSorter_Pixel_t *&output_pixels) {
  std::fprintf(stderr, "sort here\n");
  /*
   * For each line:
   *  while out of bounds: move along line
   *  while in bounds:
   *     + While in bounds
   *    * if pixel outside range
   *      - if last was in range:
   *        - place sorted pixels onto output
   *      - copy pixel from input to output
   *    * if pixel in range
   *      - add pixel to value map
   *    * move to next spot
   *  + if was in range
   *    * place sorted pixels onto output
   */
}

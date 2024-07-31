#include "PixelSorter.hpp"
#include "SDL_surface.h"
#include "global.h"
#include <cstdio>

void PixelSorter::sort(PixelSorter_Pixel_t *&input_pixels,
                       PixelSorter_Pixel_t *&output_pixels, point_ints *points,
                       int numPoints, int width, int height, int startX,
                       int startY, int endX, int endY, int deltaX, int deltaY,
                       SDL_Surface *input_test) {

  /*
   * Now in the perspective of 1 dimension L where L is the dimension with
   * greater change, that is:
   *   if |deltaX| >= |deltaY|, L = X.
   *   if |deltaX| <  |deltaY|, L - Y.
   *
   * There is also another dimension S where S is the opposite of L, that is:
   *   if L = X, S = Y.
   *   If L = Y, S = X
   */

  // Use pointers to save on lines of code
  int *startL = NULL;
  int *endL = NULL;
  int *deltaL = NULL;
  int *deltaS = NULL;

  // Change deltaL by deltaS where S is the dimension with a smaller delta
  // without changing where the center of the N lines are
  if (std::abs(deltaX) >= std::abs(deltaY)) { // X changes more than Y or same
    startL = &startX;
    endL = &endX;
    deltaL = &deltaX;
    deltaS = &deltaY;
  } else { // Y changes more than X
    startL = &startY;
    endL = &endY;
    deltaL = &deltaY;
    deltaS = &deltaX;
  }
  /* TODO: Disable after getting sweeping across L to work
  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = ((*deltaL >= 0) ? *deltaS : -*deltaS);
  (*startL) += offset;
  (*endL) -= offset;
  */
  
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
  /* TEST CODE START ======================================================== */
  // Test code to check each point in line by drawing it to the screen

  for (int i = 0; i < numPoints; i++) {
    int x = points[i].first + startX;
    int y = points[i].second + startY;

    if (0 <= x && x < width && 0 <= y && y < height) { // if in bounds
      output_pixels[TWOD_TO_1D(x, y, width)] = SDL_MapRGB(
          input_test->format, 255 * (numPoints - i) / numPoints, 0, 0);
    }
  }
  /* TEST CODE END  ========================================================= */
}

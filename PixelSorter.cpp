#include "PixelSorter.hpp"
#include "SDL_surface.h"
#include "global.h"
#include <algorithm>
#include <cstdio>

void PixelSorter::sort(PixelSorter_Pixel_t *&input_pixels,
                       PixelSorter_Pixel_t *&output_pixels, point_ints *points,
                       int numPoints, int width, int height, int startX,
                       int startY, int endX, int endY,
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

  int deltaX = endX - startX;
  int deltaY = endY - startY;

  // Use pointers to save on lines of code
  int *minL = NULL;   // The minimum value of L to sort with
  int *maxL = NULL;   // The maximum value of L to sort with
  int *deltaL = NULL; // The max of deltaX deltaY
  int *deltaS = NULL; // The min of deltaX deltaY
  // The two dimension indexes
  // int l = 0, s = 0;
  int *l = NULL, *s = NULL;

  // Change deltaL by deltaS where S is the dimension with a smaller delta
  // without changing where the center of the N lines are
  if (std::abs(deltaX) <= std::abs(deltaY)) { // X changes more than Y or same
    // Go from startX to endX or endX to startX, whichever way means going from
    // min to max
    minL = (startX <= endX) ? &startX : &endX;
    maxL = (startX <= endX) ? &endX : &startX;
    deltaL = &deltaX;
    deltaS = &deltaY;
    s = &startY;
  } else { // Y changes more than X
    minL = (startY <= endY) ? &startY : &endY;
    maxL = (startY <= endY) ? &endY : &startY;
    deltaL = &deltaY;
    deltaS = &deltaX;
    s = &startX;
  }
  l = minL;
  
  /* TODO: Disable after getting sweeping across L to work
  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = ((*deltaL >= 0) ? *deltaS : -*deltaS);
  (*startL) += offset;
  (*endL) -= offset;
  */

  printf("start(%d %d) | end(%d %d) | l m%d M%d D%d | s%d D%d\n", startX,
         startY, endX, endY, *minL, *maxL, *deltaL, *s, *deltaS);
  // For each line along l, increase it by 1
  for (l = minL; *l < *maxL; (*l)++) {
    printf("  start(%d %d) end(%d %d)\n", startX, startY, endX, endY);
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
    /* TEST CODE START ====================================================== */
    // Test code to check each point in line by drawing it to the screen

    for (int i = 0; i < numPoints; i++) {
      int x = points[i].first + startX;
      int y = points[i].second + startY;

      if (0 <= x && x < width && 0 <= y && y < height) { // if in bounds
        output_pixels[TWOD_TO_1D(x, y, width)] = SDL_MapRGB(
            input_test->format, 255 * (numPoints - i) / numPoints, 0, 0);
      }
    }
    /* TEST CODE END  ======================================================= */
  }
}

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
  int deltaX = endX - startX;
  int deltaY = endY - startY;

  // Forward decleration
  int x = 0;
  int y = 0;

  /*
   * Now in the perspective of 1 dimension L where L is the dimension with
   * less change, that is:
   *   if |deltaX| <= |deltaY|, L = X.
   *   if |deltaX| >  |deltaY|, L = Y.
   *
   * There is also another dimension S where S is the opposite of L, that is:
   *   if L = X, S = Y.
   *   If L = Y, S = X
   */
  int minL = 0; // The minimum value of L to sort with
  int maxL = 0; // The maximum value of L to sort with
  // Use pointers to save on lines of code
  int *deltaL = NULL; // The max of deltaX deltaY
  int *deltaS = NULL; // The min of deltaX deltaY
  // The index of the current line along the L dimension
  int *l = NULL;

  if (std::abs(deltaX) <= std::abs(deltaY)) { // X changes less or same as Y
    l = &x;
    maxL = width;
    deltaL = &deltaX;
    deltaS = &deltaY;
    // Offset starting y to the appropriate side, given the lines direction
    y = (deltaY >= 0) ? 0 : height - 1;
  } else { // Y changes less than X
    l = &y;
    maxL = height;
    deltaL = &deltaY;
    deltaS = &deltaX;
    // Offset starting x to the appropriate side, given the lines direction
    x = (deltaX >= 0) ? 0 : width - 1;
  }

  /* TODO: Disable after getting sweeping across L to work
  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = std::abs(*deltaS);
  (*minL) -= offset;
  (*maxL) += offset;
  */

  printf("start(%d %d) | end(%d %d) | l m%d M%d D%d | s D%d\n", startX, startY,
         endX, endY, minL, maxL, *deltaL, *deltaS);
  // For each line along l, increase it by 1
  for (; *l < maxL; (*l)++) {
    printf("%d %d\n", x, y);

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

      int px = points[i].first + x;
      int py = points[i].second + y;
      printf("(%d %d)\n", px, py);
      if (0 <= px && px < width && 0 <= py && py < height) { // if in bounds
        output_pixels[TWOD_TO_1D(px, py, width)] =
            SDL_MapRGB(input_test->format, 255 * (numPoints - i) / numPoints,
                       255 * (maxL - *l) / maxL, 0);
      }
    }
    /* TEST CODE END  ======================================================= */
  }
}

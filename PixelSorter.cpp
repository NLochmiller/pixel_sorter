#include "PixelSorter.hpp"
#include "ColorConversion.hpp"
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "global.h"
#include <algorithm>
#include <cstdio>

// Private helper to sort an individual line
void sortEachLine(PixelSorter_Pixel_t *&input_pixels,
                  PixelSorter_Pixel_t *&output_pixels, point_ints *points,
                  int numPoints, int width, int height, int deltaX, int deltaY,
                  int offsetX, int offsetY, SDL_Surface *input_test,
                  int goff = 0) {
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

  // Test code to check each point in line by drawing it to the screen

  // Starting index of the current band of sortable values
  int bandI = 0;
  uint8_t r, g, b;
  ColorConverter *converter = &ColorConversion::lightness;

  for (int i = 0; i < numPoints; i++) {
    int x = points[i].first + offsetX;
    int y = points[i].second + offsetY;
    if (!(0 <= x && x < width && 0 <= y && y < height)) {
      continue; // point is out of bounds, move on
    }
    // Point must be in bounds
    int pixelIndex = TWOD_TO_1D(x, y, width); //
    PixelSorter_Pixel_t pixel = input_pixels[pixelIndex];

    SDL_GetRGB(pixel, input_test->format, &r, &g, &b);

    // Divide by 255 to fit into the 0 to 1 range expected by converters
    double percent = converter(r / 255.0, g / 255.0, b / 255.0);

    if (percent < 0 || percent > 1) {
      fprintf(stderr, "bad percent at (%d, %d), rgb %d %d %d, p %f\n", x, y, r,
              g, b, percent);
    }

    // output_pixels[pixelIndex] =
    // SDL_MapRGB(input_test->format, 255 * (numPoints - i) / numPoints, g, 0);
  }
  printf("good\n");
}

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

  // TODO: Disable after getting sweeping across L to work
  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = std::abs(*deltaS);
  minL -= offset;
  maxL += offset;

  // For each line along l, increase it by 1
  for (*l = minL; *l < maxL; (*l)++) {
    sortEachLine(input_pixels, output_pixels, points, numPoints, width, height,
                 deltaX, deltaY, x, y, input_test,
                 255 * (maxL + minL - *l) / maxL);
  }
}

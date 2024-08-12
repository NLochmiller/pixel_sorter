#include "PixelSorter.hpp"
#include "ColorConversion.hpp"
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "global.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// How many unique values can there be, also how precise are our values
#define COUNT_T long

#define BOOL2STR(_bool_) (_bool_) ? "true" : "false"

// indexes

void sortBand(PixelSorter_Pixel_t *&input_pixels,
              PixelSorter_Pixel_t *&output_pixels, PixelSorter_value_t *values,
              int *pixelIndexes, int numPoints, int width, int height,
              int bandStartIndex, int bandEndIndex) {
  static const COUNT_T countLen = PRECISION + 1;
  // Count will store the count of each number
  COUNT_T *count = (COUNT_T *)calloc(countLen, sizeof(COUNT_T));
  if (count == NULL) {
    fprintf(stderr, "Could not create a count array. Image may be too big\n");
    exit(-3);
  }
  COUNT_T lineIndex = bandStartIndex;
  // printf("line index = %d\n", bandStartIndex);
  // printf("pixelIndexes %s\n", BOOL2STR(pixelIndexes == NULL));
  // printf("values null? %s\n", BOOL2STR(values == NULL));
  // Store count of each value
  for (lineIndex = bandStartIndex; lineIndex < bandEndIndex; lineIndex++) {
    int pixelIndex = pixelIndexes[lineIndex];
    PixelSorter_value_t value = values[pixelIndex];
    (count[value])++;
  }

  // Change count[i] so that count[i] now contains actual
  // position of this value in output surface
  for (int i = 1; i <= PRECISION; i++) {
    count[lineIndex] += count[lineIndex - 1];
  }

  for (lineIndex = bandStartIndex; lineIndex < bandEndIndex; lineIndex++) {
    int pixelIndex = pixelIndexes[lineIndex];

    // outPixels[w*(n - (count[values[i * w]]))] = inPixels[i];

    int x = pixelIndex % width;
    int y = pixelIndex / width;
    if (!(x >= 0 && x < width && y >= 0 && y < height)) {
      fprintf(stderr, "BAD COORDS %d %d\n", x, y);
    }
    printf("\t\tindex[%d] = (%d,%d) = %d\n", pixelIndex, x, y, values[pixelIndex]);

    int outputLineIndex = lineIndex + (count[values[pixelIndex]] - 1);
    output_pixels[pixelIndexes[outputLineIndex]] = input_pixels[pixelIndex];
    (count[values[pixelIndex]])--;
    // count[values[]]
    // output_pixels[count[values[lineIndex]]] = input_pixels[]
    // count[values[i* w]]--;

    // Copy each pixel
    // output_pixels[pixelIndex] = input_pixels[pixelIndex];
  }
  free(count);
}

// Private helper to sort an individual line
void sortEachLine(PixelSorter_Pixel_t *&input_pixels,
                  PixelSorter_Pixel_t *&output_pixels, point_ints *points,
                  int numPoints, int width, int height, int deltaX, int deltaY,
                  int offsetX, int offsetY, int valueMin, int valueMax,
                  SDL_Surface *input_test, int goff = 0) {
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
  int bandStartIndex = 0;
  uint8_t r, g, b;
  // TODO: make a variable
  ColorConverter *converter = &(ColorConversion::maximum);
  PixelSorter_value_t values[width * height]; // pixelIndex to value
  bool wasLastInBand = false;                 // if the last pixel was in a band
  // Conversion map from lineIndex to pixelIndex-
  int *pixelIndexes = (int *)calloc(numPoints, sizeof(int));

  for (int lineIndex = 0; lineIndex < numPoints; lineIndex++) {
    int x = points[lineIndex].first + offsetX;
    int y = points[lineIndex].second + offsetY;
    int pixelIndex = TWOD_TO_1D(x, y, width);

    pixelIndexes[lineIndex] = pixelIndex;
    if (!(0 <= x && x < width && 0 <= y && y < height)) { // Check for outside
      if (wasLastInBand) {
        // TODO: Sort from bandStartIndex to lineIndex - 1
        printf("\texit band, leaving image\n");
        sortBand(input_pixels, output_pixels, values, pixelIndexes, numPoints,
                 width, height, bandStartIndex, lineIndex);
      }
      wasLastInBand = false;
      continue; // point is out of bounds, move on to next point
    }
    /* Point must be in bounds */

    PixelSorter_Pixel_t pixel = input_pixels[pixelIndex];
    SDL_GetRGB(pixel, input_test->format, &r, &g, &b);
    // Divide by 255 to fit into the 0 to 1 range expected by converters
    PixelSorter_value_t percent = std::round(
        PRECISION * converter(((double)r) / 255.0, ((double)g) / 255.0,
                              ((double)b) / 255.0));
    if (percent < 0 || percent > PRECISION) { // Sanity check
      fprintf(stderr, "Bad percent at (%d, %d), rgb %d %d %d, p %f, %d/%d\n", x,
              y, r, g, b, 1.0f * percent / PRECISION, percent, PRECISION);
    }

    // A band is a contiguous list of pixels that are within the min max values
    bool inBand = valueMin <= percent && valueMax >= percent;
    // State: out of band
    if (!inBand) {
      // Copy input to output
      output_pixels[pixelIndex] = SDL_MapRGB(input_test->format, 0, 255, 0);
      // input_pixels[pixelIndex];
      if (wasLastInBand) { // If transitioned out of a bad, sort the band
        // TODO: Sort the band from bandStartIndex to lineIndex - 1
        printf("\texit band, in image\n");
        sortBand(input_pixels, output_pixels, values, pixelIndexes, numPoints,
                 width, height, bandStartIndex, lineIndex);
      }
      wasLastInBand = false;
    } else {
      if (!wasLastInBand) {         // If it is the start of a band
        bandStartIndex = lineIndex; // Remember starting index
      }
      // Add current pixel value to values
      values[pixelIndex] = percent;
      wasLastInBand = true;
    }
  }
  // If was in a band at the end of the line, we must sort
  if (wasLastInBand) {
    // TODO: Sort from bandStartIndex to numPoints - 1
    printf("\texit band, last pixel\n");
    sortBand(input_pixels, output_pixels, values, pixelIndexes, numPoints,
             width, height, bandStartIndex, numPoints - 1);
  }
  free(pixelIndexes);
}

void PixelSorter::sort(PixelSorter_Pixel_t *&input_pixels,
                       PixelSorter_Pixel_t *&output_pixels, point_ints *points,
                       int numPoints, int width, int height, int startX,
                       int startY, int endX, int endY, double valueMin,
                       double valueMax, SDL_Surface *input_test) {
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
  int *deltaS = NULL; // The min of deltaX deltaY
  // The index of the current line along the L dimension
  int *l = NULL;

  if (std::abs(deltaX) <= std::abs(deltaY)) { // X changes less or same as Y
    l = &x;
    maxL = width;
    deltaS = &deltaY;
    // Offset starting y to the appropriate side, given the lines direction
    y = (deltaY >= 0) ? 0 : height - 1;
  } else { // Y changes less than X
    l = &y;
    maxL = height;
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
    printf("Line # %d\n", (*l));
    sortEachLine(input_pixels, output_pixels, points, numPoints, width, height,
                 deltaX, deltaY, x, y, valueMin * PRECISION,
                 valueMax * PRECISION, input_test,
                 255 * (maxL + minL - *l) / maxL);
  }
}

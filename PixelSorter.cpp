#include "PixelSorter.hpp"
#include "ColorConversion.hpp"
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "global.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>

// How many unique values can there be, also how precise are our values
#define COUNT_T long

// Sort a band of pixels.
void sortBand(PixelSorter_Pixel_t *&input_pixels,
              PixelSorter_Pixel_t *&output_pixels, PixelSorter_value_t *values,
              int *pixelIndexes, int numPoints, int width, int height,
              int bandStartIndex, int bandEndIndex) {
  static const COUNT_T countLen = PRECISION + 1;
  // Count will store the count of each number
  COUNT_T *count = (COUNT_T *)calloc(countLen, sizeof(COUNT_T));
  if (count == NULL) {
    fprintf(stderr, "Could not create a count array. Image may be too big\n");
    return;
  }
  COUNT_T lineIndex = bandStartIndex;

  // Count each value
  for (lineIndex = bandStartIndex; lineIndex < bandEndIndex; lineIndex++) {
    int pixelIndex = pixelIndexes[lineIndex];
    PixelSorter_value_t value = values[pixelIndex];
    (count[value])++;
  }

  // Change count[i] so that count[i] now contains actual
  // position of this value in output surface
  for (int i = 1; i <= PRECISION; i++) {
    (count[i]) += (count[i - 1]);
  }

  // Load output with the intended inputs
  for (lineIndex = bandStartIndex; lineIndex < bandEndIndex; lineIndex++) {
    int pixelIndex = pixelIndexes[lineIndex]; // Pixel index of lineIndex
    // The line index that the output pixel is at
    int outputLineIndex = bandStartIndex + (count[values[pixelIndex]] - 1);
    output_pixels[pixelIndexes[outputLineIndex]] = input_pixels[pixelIndex];
    (count[values[pixelIndex]])--;
  }
  free(count);
}

// Private helper to sort an individual line
bool sortEachLine(PixelSorter_Pixel_t *&input_pixels,
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

  // Starting index of the current band of sortable values
  int bandStartIndex = 0;
  uint8_t r, g, b; // Individual color values, that will be used later
  // TODO: make a variable
  ColorConverter *converter = &(ColorConversion::maximum);
  PixelSorter_value_t values[width * height]; // pixelIndex to value
  bool wasLastInBand = false;                 // if the last pixel was in a band
  // Conversion map from lineIndex to pixelIndex-
  int *pixelIndexes = (int *)calloc(numPoints, sizeof(int));

  int lineIndex = 0;

  // Advance until lineIndex is actually in the image
  for (; lineIndex < numPoints; lineIndex++) {
    int x = points[lineIndex].first + offsetX;
    int y = points[lineIndex].second + offsetY;
    if (0 <= x && x < width && 0 <= y && y < height) {
      break; // We are inside the image, get to it
    }
  }

  if (lineIndex >= numPoints) {
    return false; // reached numPoints, thus band does not touch image, stop
  }

  // Loop until we exit the image, or line goes past the image
  for (; lineIndex < numPoints; lineIndex++) {
    int x = points[lineIndex].first + offsetX;
    int y = points[lineIndex].second + offsetY;
    int pixelIndex = TWOD_TO_1D(x, y, width);

    pixelIndexes[lineIndex] = pixelIndex;
    if (!(0 <= x && x < width && 0 <= y && y < height)) { // Check for outside
      if (wasLastInBand) {
        // Sort from bandStartIndex to lineIndex
        sortBand(input_pixels, output_pixels, values, pixelIndexes, numPoints,
                 width, height, bandStartIndex, lineIndex);
      }
      wasLastInBand = false;
      break; // point is out of bounds, no more points to read
    }

    /* Point must be in bounds at this point */
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
      output_pixels[pixelIndex] = input_pixels[pixelIndex];
      if (wasLastInBand) { // If transitioned out of a bad, sort the band
        // Sort the band from bandStartIndex to lineIndex - 1
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
    // Sort from bandStartIndex to numPoints - 1
    sortBand(input_pixels, output_pixels, values, pixelIndexes, numPoints,
             width, height, bandStartIndex, numPoints - 1);
  }
  free(pixelIndexes);
  return true;
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

  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = std::abs(*deltaS);
  minL -= offset;
  maxL += offset;

  bool endedInBounds = false; // Did the last band end in bounds?
  // Go through each empty line (go until we hit the image)
  for (*l = minL; *l < maxL && !endedInBounds; (*l)++) {
    endedInBounds = sortEachLine(input_pixels, output_pixels, points, numPoints,
                                 width, height, deltaX, deltaY, x, y,
                                 valueMin * PRECISION, valueMax * PRECISION,
                                 input_test, 255 * (maxL + minL - *l) / maxL);
  }

  // For each line along l, increase it by 1
  for (; *l < maxL && endedInBounds; (*l)++) {
    endedInBounds = sortEachLine(input_pixels, output_pixels, points, numPoints,
                                 width, height, deltaX, deltaY, x, y,
                                 valueMin * PRECISION, valueMax * PRECISION,
                                 input_test, 255 * (maxL + minL - *l) / maxL);
  }
}

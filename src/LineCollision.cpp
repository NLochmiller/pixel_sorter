#include "LineCollision.hpp"
#include "LineInterpolator.hpp"
#include <cmath>
#include <cstdio>

using LineCollision::point_doubles;
using LineCollision::point_ints;
using LineCollision::pointQueue;

// Return the intersection of the line from x,y to the center of min[XY] max[XY]
point_doubles LineCollision::pointOnRect(double x, double y, double minX,
                                         double maxX, double minY,
                                         double maxY) {
  double midX = (minX + maxX) / 2;
  double midY = (minY + maxY) / 2;
  double m = (midY - y) / (midX - x);

  if (x <= midX) { // check "left" side
    double minXy = m * (minX - x) + y;
    if (minY <= minXy && minXy <= maxY)
      return std::make_pair(minX, minXy);
  }

  if (x >= midX) { // check "right" side
    double maxXy = m * (maxX - x) + y;
    if (minY <= maxXy && maxXy <= maxY)
      return std::make_pair(maxX, maxXy);
  }

  if (y <= midY) { // check "top" side
    double minYx = (minY - y) / m + x;
    if (minX <= minYx && minYx <= maxX)
      return std::make_pair(minYx, minY);
  }

  if (y >= midY) { // check "bottom" side
    double maxYx = (maxY - y) / m + x;
    if (minX <= maxYx && maxYx <= maxX)
      return std::make_pair(maxYx, maxY);
  }
  // edge case when finding midpoint intersection: m = 0/0 = NaN
  if (x == midX && y == midY) {
    return std::make_pair(0.0, 0.0);
  }

  fprintf(stderr, "pointOnRect: UNACCOUNTED CASE\n"); // Error print.
  return std::make_pair(0.0, 0.0);
}

// Generate a Bresenham's line at angle that goes from origin to any edge of the
// rectangle. With the origin being (0, 0), the line starts at the origin, and
// the rectangle is centered on the origin
pointQueue LineCollision::generateLineQueueForRect(double &angle, int width,
                                                   int height,
                                                   BresenhamsArguments &args) {
  if (angle == 360) {
    angle = 0;
  }
  /* Calculate end point */
  double angleInRads = angle * (M_PI / 180.0f);
  double lineLength = width * width + height * height;
  point_doubles smallLine = std::make_pair(lineLength * std::cos(angleInRads),
                                           lineLength * std::sin(angleInRads));

  // maximum dimension
  double maxD = std::abs((std::abs(width) > std::abs(height)) ? width : height);
  // Find the point on rect that is centered on origin, where a line can be
  // drawn to it from origin with given angle from 0 degrees
  point_doubles endPoint =
      pointOnRect(smallLine.first, smallLine.second, -maxD, maxD, -maxD, maxD);

  /* Initalize and get line interpolator */
  args.init(0, 0, (int)std::round(endPoint.first),
            (int)std::round(endPoint.second));
  bresenham_interpolator *interpolator =
      LineInterpolator::get_interpolator(args.deltaX, args.deltaY);

  /* Fill queue with points on line */
  pointQueue points;
  do {
    points.push(std::make_pair(args.currentX, args.currentY));
  } while (interpolator(args));
  return points;
}

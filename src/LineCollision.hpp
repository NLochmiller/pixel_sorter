#ifndef LINECOLLISION_HPP_
#define LINECOLLISION_HPP_

/*
 * Calculate the line that would collide with a given rectangle
 */
#include "LineInterpolator.hpp"
#include <queue>
#include <utility>
namespace LineCollision {

// typedef struct {
//   double x;
//   double y;
// } point_doubles; // Double point

// using point_doubles =
typedef std::pair<double, double> point_doubles;
typedef std::pair<int, int> point_ints;
typedef std::queue<point_ints> pointQueue;

point_doubles pointOnRect(double x, double y, double minX, double maxX,
                          double minY, double maxY);
pointQueue generateLineQueueForRect(double &angle, int width, int height,
                                    BresenhamsArguments &args);
} // namespace Rename

#endif //  LINECOLLISION_HPP_

#include "bresenhamsLine_Interpolator.hpp"
#include <cstdio>

/*
 * === ANGLES ==================================================================
 * Angles are from the right side, starting at 0 degrees up to 360 degrees
 * ┌───────────────┐
 * │      90       │
 * │      |        │
 * │      |        │
 * │      |        │
 * │180---*---0/360│
 * │      |        │
 * │      |        │
 * │      |        │
 * │     270       │
 * └───────────────┘
 *
 * === OCTANTS =================================================================
 * Octants are divided into these groups (Starting from right going
 * counterclockwise)
 *
 * ┌─────────┐
 * │         │
 * │ \  |  / │
 * │  \2|1/  │
 * │  3\|/0  │
 * │ ---*--- │
 * │  4/|\7  │
 * │  /5|6\  │
 * │ /  |  \ │
 * │         │
 * └─────────┘
 */

bool LineInterpolator::inerpolate_bresenhams(int &currentX, int &currentY,
                                             int startX, int startY, int endX,
                                             int endY, int &dx, int &dy,
                                             double &slope_error) {
  /*
  if (slope_error > 0) {
    currentY++;
    slope_error -= dx;
  }
  slope_error += 2 * dy;
  */
  /* Unmodifed bresenhams
  for (; currentX <= endX; currentX++) {
    printf("(%d, %d)\n", currentX, currentY);
    if (slope_error > 0) {
      currentY++;
      slope_error -= 2 * dx;
    }
    slope_error += 2 * dy;
  }
  */

  if (slope_error > 0) {
    currentY++;
    slope_error -= 2 * dx;
  }
  slope_error += 2 * dy;
  currentX++;

  return (currentX <= endX);
}

void LineInterpolator::init_bresenhams(int &currentX, int &currentY, int startX,
                                       int startY, int endX, int endY, int &dx,
                                       int &dy, double &slope_error) {
  currentX = startX;
  currentY = startY;

  dx = endX - startX;
  dy = endY - startY;

  slope_error = 2 * dy - dx;
}

#include "bresenhamsLine_Interpolator.hpp"

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

/*
  Have a function that returns an interpolator, something like get_interpolator
 */

bool LineInterpolator::interpolate_bresenhams(int &currentX, int &currentY,
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

// Returns a bresenhams line algorithm interpolator based on angle in degrees
bresenham_interpolator* LineInterpolator::get_interpolator(double angle) {
  return &(LineInterpolator::interpolate_bresenhams_O0);
}

bool LineInterpolator::interpolate_bresenhams_O0(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY--;
    slope_error -= 2 * dx;
  }
  slope_error += 2 * dy;
  currentX++;

  return (currentX <= endX);
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O1(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O2(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O3(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O4(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O5(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O6(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return true;
}

bool LineInterpolator::interpolate_bresenhams_O7(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY++;
    slope_error -= 2 * dx;
  }
  slope_error += 2 * dy;
  currentX++;

  return (currentX <= endX);
}

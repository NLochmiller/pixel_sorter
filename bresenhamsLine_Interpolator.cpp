#include "bresenhamsLine_Interpolator.hpp"
#include <cstdio>
#include <cstdlib>

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
 * Initalize the variables used in bresenhams line interpolation.
 * You must provide all variables, the only variables who need to have a value
 * are startX, startY, endX, and endY, all other variables will be assigned
 * values when init_bresenhams is called.
 */
void LineInterpolator::init_bresenhams(int &currentX, int &currentY, int startX,
                                       int startY, int endX, int endY, int &dx,
                                       int &dy, double &slope_error) {
  currentX = startX;
  currentY = startY;

  dx = endX - startX;
  dy = endY - startY;

  slope_error = 2 * dy - dx;
}

/*
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
bresenham_interpolator *LineInterpolator::get_interpolator(int dx, int dy) {

  int abs_dx = std::abs(dx);
  int abs_dy = std::abs(dy);

  if (dx > 0 && dy >= 0) { // Top right quadrant
    if (abs_dx > abs_dy) {
      return &LineInterpolator::interpolate_bresenhams_O0;

    } else {
      return &LineInterpolator::interpolate_bresenhams_O1;
    }
  } else if (dx <= 0 && dy > 0) { // Top left quadrant
    if (abs_dx < abs_dy) {        // [90, 135)
      return &LineInterpolator::interpolate_bresenhams_O2;
    } else {
      return &LineInterpolator::interpolate_bresenhams_O3;
    }
  } else if (dx < 0 && dy <= 0) { // Bottom left quadrant
    if (abs_dx >= abs_dy) {
      return &LineInterpolator::interpolate_bresenhams_O4;
    } else {
      return &LineInterpolator::interpolate_bresenhams_O5;
    }
  } else if (dx >= 0 && dy < 0) { // Bottom right quadrent
    if (abs_dx < abs_dy) {
      return &LineInterpolator::interpolate_bresenhams_O6;
    } else {
      return &LineInterpolator::interpolate_bresenhams_O7;
    }
  }
  fprintf(stderr, "LineInterpolator::get_interpolator Invalid dx dy (%d, %d)\n",
          dx, dy);

  return &LineInterpolator::interpolate_bresenhams_O0;
}

// Returns a bresenhams line algorithm interpolator based on angle in degrees
// Angle must be in range [0, 360)
bresenham_interpolator *LineInterpolator::get_interpolator(double angle) {
  if ((angle >= 0.0f && angle < 45.0f) || angle == 360.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O0);
  } else if (angle >= 45.0f && angle < 90.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O1);
  } else if (angle >= 90.0f && angle < 135.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O2);
  } else if (angle >= 135.0f && angle < 180.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O3);
  } else if (angle >= 180.0f && angle < 225.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O4);
  } else if (angle >= 225.0f && angle < 270.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O5);
  } else if (angle >= 270.0f && angle < 315.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O6);
  } else if (angle >= 315.0f && angle < 360.0f) {
    return &(LineInterpolator::interpolate_bresenhams_O7);
  }
  // Default to octant 0
  return &(LineInterpolator::interpolate_bresenhams_O0);
}

// Return false always, indicating done
bool LineInterpolator::invalid_bresenhams_interpolator(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  return false;
}

// Interpolate octant 0
bool LineInterpolator::interpolate_bresenhams_O0(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY++; // Go up
    slope_error -= 2 * std::abs(dx);
  }
  slope_error += 2 * std::abs(dy);
  currentX++; // Head right

  return (currentX <= endX);
}

// Interpolate octant 1. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O1(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentX++; // Right
    slope_error -= 2 * std::abs(dy);
  }
  slope_error += 2 * std::abs(dx);
  currentY++; // Up

  return (currentY <= endY);
}

// Interpolate octant 2. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O2(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentX--; // Left
    slope_error += 2 * std::abs(dy);
  }
  slope_error -= 2 * std::abs(dx);
  currentY++; // Up

  return (currentY <= endY);
}

// Interpolate octant 3. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O3(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY++; // Go up
    slope_error -= 2 * std::abs(dx);
  }
  slope_error += 2 * std::abs(dy);
  currentX--; // Go left

  return (currentX >= endX);
}

// Interpolate octant 4. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O4(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentY--; // Go down
    slope_error += 2 * std::abs(dx);
  }
  slope_error -= 2 * std::abs(dy);
  currentX--; // Go left

  return (currentX >= endX);
}

// Interpolate octant 5. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O5(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentX--; // Left
    slope_error += 2 * std::abs(dy);
  }
  slope_error -= 2 * std::abs(dx);
  currentY--; // Down

  return (currentY >= endY);
}

// Interpolate octant 6. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O6(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentX++; // Right
    slope_error -= 2 * std::abs(dy);
  }
  slope_error += 2 * std::abs(dx);
  currentY--; // Down

  return (currentY >= endY);
}

// Interpolate octant 7. See octant 0 for more information
bool LineInterpolator::interpolate_bresenhams_O7(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentY--; // Go down
    slope_error += 2 * std::abs(dx);
  }
  slope_error -= 2 * std::abs(dy);
  currentX++; // Go right

  return (currentX <= endX);

  return false;
}

/* NOTE:
 * Use this if you do not care about performance, if you care about performance,
 * call get_interpolator and use what it returns in place of this
 *
 * MUST BE CALLED AFTER init_bresenhams WITH VARIABLES PASSED TO IT
 * Returns if you are at the end of the line.
 */
bool LineInterpolator::interpolate_bresenhams(int &currentX, int &currentY,
                                              int startX, int startY, int endX,
                                              int endY, int &dx, int &dy,
                                              double &slope_error) {
  // Get appropriate octant function and return its value
  bresenham_interpolator *func = get_interpolator(dx, dy);
  return func(currentX, currentY, startX, startY, endX, endY, dx, dy,
              slope_error);
}

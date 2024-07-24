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
  // Normalize
  // dx = std::abs(dx);
  // dy = std::abs(dy);
}

// Returns a bresenhams line algorithm interpolator based on angle in degrees
// Angle must be in range [0, 360)
bresenham_interpolator *LineInterpolator::get_interpolator(double angle) {
  if ((angle >= 0.0f && angle < 45.0f) || angle == 360.0f) {
    printf("O0 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O0);
  } else if (angle >= 45.0f && angle < 90.0f) {
    printf("O1 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O1);
  } else if (angle >= 90.0f && angle < 135.0f) {
    printf("O2 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O2);
  } else if (angle >= 135.0f && angle < 180.0f) {
    printf("O3 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O3);
  } else if (angle >= 180.0f && angle < 225.0f) {
    printf("O4 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O4);
  } else if (angle >= 225.0f && angle < 270.0f) {
    printf("O5 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O5);
  } else if (angle >= 270.0f && angle < 315.0f) {
    printf("O6 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O6);
  } else if (angle >= 315.0f && angle < 360.0f) {
    printf("O7 %f\n", angle);
    return &(LineInterpolator::interpolate_bresenhams_O7);
  }
  // Default to octant 0
  return &(LineInterpolator::interpolate_bresenhams_O0);
}

bool LineInterpolator::interpolate_bresenhams_O0(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY++; // Go up
    slope_error -= 2 * std::abs(dx);
  }
  slope_error += 2 * std::abs(dy);
  currentX++;  // Head right

  return (currentX <= endX);
}

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

bool LineInterpolator::interpolate_bresenhams_O3(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error > 0) {
    currentY++; // Go up
    slope_error -= 2 * std::abs(dx);
  }
  slope_error += 2 * std::abs(dy);
  currentX--;  // Go left

  return (currentX >= endX);
}

bool LineInterpolator::interpolate_bresenhams_O4(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentY--; // Go down
    slope_error += 2 * std::abs(dx);
  }
  slope_error -= 2 * std::abs(dy);
  currentX--;  // Go left
  
  return (currentX >= endX);
}

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

bool LineInterpolator::interpolate_bresenhams_O7(
    BRESENHAMS_INTERPOLATOR_ARGUMENTS) {
  if (slope_error < 0) {
    currentY--; // Go down
    slope_error += 2 * std::abs(dx);
  }
  slope_error -= 2 * std::abs(dy);
  currentX++;  // Go right
  
  return (currentX <= endX);

  return false;
}

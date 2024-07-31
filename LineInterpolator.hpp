#ifndef BRESENHAMSLINE_INTERPOLATOR_HPP_
#define BRESENHAMSLINE_INTERPOLATOR_HPP_

// Class used to simplify passing arguments to bresenhams interpolators
class BresenhamsArguments {
public:
  // Constructor
  BresenhamsArguments(int startX = 0, int startY = 0, int endX = 0,
                      int endY = 0);
  // Allow for reusing the arguments, or calculating arguments after creation
  void init(int startX, int startY, int endX, int endY);
  // Variables
  int currentX;
  int currentY;
  int startX;
  int startY;
  int endX;
  int endY;
  int deltaX;
  int deltaY;
  double slope_error;
};

#define BRESENHAMS_INTERPOLATOR_ARGS BresenhamsArguments &args

// Type for interpolator functions
typedef bool bresenham_interpolator(BresenhamsArguments &);

// Constants for different octants
#define LINEINTERPOLATOR_OCTANT_0 0
#define LINEINTERPOLATOR_OCTANT_1 1
#define LINEINTERPOLATOR_OCTANT_2 2
#define LINEINTERPOLATOR_OCTANT_3 3
#define LINEINTERPOLATOR_OCTANT_4 4
#define LINEINTERPOLATOR_OCTANT_5 5
#define LINEINTERPOLATOR_OCTANT_6 6
#define LINEINTERPOLATOR_OCTANT_7 7

class LineInterpolator {
public:
  // Constructor, does nothing since we only have static methods
  LineInterpolator() {}

  // Get interpolator for use case
  static bresenham_interpolator *get_interpolator(int dx, int dy);
  static bresenham_interpolator *get_interpolator(double angle);

  /*
   * A interpolator that acts like NULL, it changes nothing and returns false.
   * This is intended to be passed when invalid input is given, if code is
   * correctly setup, the loop with bad input should stop
   */
  static bresenham_interpolator invalid_bresenhams_interpolator;

  // One interpolator for each octant.
  static bresenham_interpolator interpolate_bresenhams_O0;
  static bresenham_interpolator interpolate_bresenhams_O1;
  static bresenham_interpolator interpolate_bresenhams_O2;
  static bresenham_interpolator interpolate_bresenhams_O3;
  static bresenham_interpolator interpolate_bresenhams_O4;
  static bresenham_interpolator interpolate_bresenhams_O5;
  static bresenham_interpolator interpolate_bresenhams_O6;
  static bresenham_interpolator interpolate_bresenhams_O7;

  // Interperate along the line using bresenhams line drawing algorithm
  // returns if finished
  static bresenham_interpolator interpolate_bresenhams;
};

#endif // BRESENHAMSLINE_INTERPOLATOR_HPP_

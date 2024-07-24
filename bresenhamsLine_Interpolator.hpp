#ifndef BRESENHAMSLINE_INTERPOLATOR_HPP_
#define BRESENHAMSLINE_INTERPOLATOR_HPP_

#define BRESENHAMS_INTERPOLATOR_ARGUMENTS                                      \
  int &currentX, int &currentY, int startX, int startY, int endX, int endY,    \
      int &dx, int &dy, double &slope_error

// Internal use only
typedef bool bresenham_interpolator(int &, int &, int, int, int, int, int &,
                                    int &, double &);

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

  // Initalize variables needed by bresenhams
  // Variables user must supply: startX, startY, endX, endY
  static void init_bresenhams(int &currentX, int &currentY, int startX,
                              int startY, int endX, int endY, int &dx, int &dy,
                              double &slope_error);
  // Get interpolator for use case
  static bresenham_interpolator *get_interpolator(int dx, int dy);
  static bresenham_interpolator *get_interpolator(double angle);

  /*
   * A interpolator that acts like NULL, it changes nothing and returns false.
   * This is intended to be passed when invalid input is given, if code is
   * correctly setup, the loop with bad input should stop 
   */
  static bresenham_interpolator invalid_bresenhams_interpolator;
  
  // For each octant.
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


private:
};

// using bresenham_interpolator = bool (LineInterpolator::*)(int &, int &, int,
// int, int, int, int &, int &, double &);
typedef int (LineInterpolator::*typedefName)(int);
// typedef
// typename bresenham_interpolator;
// (int &, int &, int, int, int, int, int &, int &,
//                                double &);

#endif // BRESENHAMSLINE_INTERPOLATOR_HPP_

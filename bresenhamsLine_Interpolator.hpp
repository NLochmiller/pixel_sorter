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
  // Interperate along the line using bresenhams line drawing algorithm
  // returns if finished
  // currentX and currentY are updated to the next step in the line drawing
  // algorithm
  // static bool interpolate_bresenhams(int &currentX, int &currentY, int
  // startX,
  //                                    int startY, int endX, int endY, int &dx,
  //                                    int &dy, double &slope_error);

  static bresenham_interpolator interpolate_bresenhams;

  // Initalize variables needed by bresenhams
  // Variables user must supply: startX, startY, endX, endY
  static void init_bresenhams(int &currentX, int &currentY, int startX,
                              int startY, int endX, int endY, int &dx, int &dy,
                              double &slope_error);

  static bresenham_interpolator* get_interpolator(double angle);

  
  // For each octant.
  static bresenham_interpolator interpolate_bresenhams_O0;
  static bresenham_interpolator interpolate_bresenhams_O1;
  static bresenham_interpolator interpolate_bresenhams_O2;
  static bresenham_interpolator interpolate_bresenhams_O3;
  static bresenham_interpolator interpolate_bresenhams_O4;
  static bresenham_interpolator interpolate_bresenhams_O5;
  static bresenham_interpolator interpolate_bresenhams_O6;
  static bresenham_interpolator interpolate_bresenhams_O7;

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

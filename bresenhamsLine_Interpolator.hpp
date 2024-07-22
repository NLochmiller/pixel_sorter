#ifndef BRESENHAMSLINE_INTERPOLATOR_HPP_
#define BRESENHAMSLINE_INTERPOLATOR_HPP_

class LineInterpolator {
public:
  // Constructor, does nothing since we only have static methods
  LineInterpolator() {}
  // Interperate along the line using bresenhams line drawing algorithm
  // returns if finished
  // currentX and currentY are updated to the next step in the line drawing
  // algorithm
  static bool inerpolate_bresenhams(int &currentX, int &currentY, int startX,
                                    int startY, int endX, int endY, int &dx,
                                    int &dy, double &slope_error);

  // Initalize variables needed by bresenhams
  // Variables user must supply: startX, startY, endX, endY
  static void init_bresenhams(int &currentX, int &currentY, int startX,
                              int startY, int endX, int endY, int &dx, int &dy,
                              double &slope_error);

private:
};

#endif // BRESENHAMSLINE_INTERPOLATOR_HPP_

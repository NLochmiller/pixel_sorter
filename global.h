#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <cstdint>

// The default pixel format to be used in this program
extern const uint32_t DEFAULT_PIXEL_FORMAT; //
#define TWOD_TO_1D(_x_, _y_, _w_) _x_ + (_y_ * _w_)

#endif // GLOBAL_H_

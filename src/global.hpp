#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_

#include <cstdint>

// The default pixel format to be used in this program
extern const uint32_t DEFAULT_PIXEL_FORMAT; //
// The default pixel depth (in bits)
#define DEFAULT_DEPTH 8

// The image types supported by this program
#define SUPPORTED_IMAGE_TYPES                                                  \
  { ".png", ".jpg" }

// Macro to convert from a 2d coordinates system to 1d
#define TWOD_TO_1D(_x_, _y_, _w_) _x_ + (_y_ * _w_)

//
template< class Type, std::ptrdiff_t  n >
std::ptrdiff_t arrayLen( Type (&)[n] ) { return n; }

#endif // GLOBAL_HPP_

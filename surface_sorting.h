#ifndef SURFACE_SORTING_H
#define SURFACE_SORTING_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "constants.h"

// Sort the pixels based on the masked values
SDL_Surface* sortPixels(SDL_Surface* in, SDL_Surface* out, Uint8* values,
                        bool* valuesMask, unsigned long numPixels,
                        enum direction direction);


#endif /* SURFACE_SORTING_H */

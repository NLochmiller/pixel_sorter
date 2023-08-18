#ifndef SURFACE_SORTING_H
#define SURFACE_SORTING_H

#include <stdbool.h>
#include <SDL2/SDL.h>


SDL_Surface* sortPixels(SDL_Surface* in, SDL_Surface* out, Uint8* values,
                        bool* valuesMask, unsigned long numPixels);


#endif /* SURFACE_SORTING_H */

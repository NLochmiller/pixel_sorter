/*
 * Helper functions for dealing with ImGui with the SDL2 backend
 */

#ifndef IMGUI_SDL2_HELPERS_HPP_
#define IMGUI_SDL2_HELPERS_HPP_

#include "SDL_render.h"

// Render the entire window
void render(SDL_Renderer *renderer);

// Display the texture
// A width or height of 0 indicates that the dimension of the texture should be
// used
bool displayTexture(SDL_Renderer *renderer, SDL_Texture *texture,
                    uint width = 0, uint height = 0);

// Display the zoomable texture
bool displayTextureZoomable(SDL_Renderer *renderer, SDL_Texture *texture,
                            uint width = 0, uint height = 0,
                            float previewNum = 32, float previewSize = 4);
// Display the zoomable texture
bool displayTextureZoomable(SDL_Renderer *renderer, SDL_Texture *texture,
                            uint width = 0, uint height = 0, uint dwidth = 0,
                            uint dheight = 0, float previewNum = 32,
                            float previewSize = 4);

// For width and height, 0 indicates to use the respective dimension of the
// surface
bool displaySurface(SDL_Renderer *renderer, SDL_Surface *surface,
                    uint width = 0, uint height = 0);

// Update the texture to whatever surface is.
// texture will always be destoryed, and can be NULL
// if surface is NULL, texture is returned
SDL_Texture *updateTexture(SDL_Renderer *renderer, SDL_Surface *surface,
                           SDL_Texture *texture);

/*
 * A custom version of SDL_ConvertSurfaceFormat with the following differences:
 *  1) Does not require the unused flag parameter
 *  2) Always does the following
 *    - Creates a copy of surface in pixel_format
 *    - Frees old surface which is in the original format we don't want
 *  3) The input surface should be immediatly replaced with the returned surface
 *
 * Inputs:
 *   src - the existing SDL_Surface structure to convert.
 *   fmt - the SDL_PixelFormat structure that the new surface is optimized for.
 *
 * Output:
 *   A pointer to a copy of src in fmt. Never returns src.
 *   Can return NULL if src is NULL or another error occurs
 */
SDL_Surface *SDL_ConvertSurfaceFormat_MemSafe(SDL_Surface *src,
                                              const Uint32 fmt);

#endif // IMGUI_SDL2_HELPERS_HPP_

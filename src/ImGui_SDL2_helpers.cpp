#include "ImGui_SDL2_helpers.hpp"
#include "ImGui_ImageZoomable.hpp"
#include "imgui.h"
#include "imgui_impl_sdlrenderer2.h"

// Render the entire window
void render(SDL_Renderer *renderer) {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::Render();
  SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x,
                     io.DisplayFramebufferScale.y);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}

// Display the texture
// A width or height of 0 indicates that the dimension of the texture should be
// used
bool displayTexture(SDL_Renderer *renderer, SDL_Texture *texture, uint width,
                    uint height) {
  // Get dimensions of the texture only if either width or height is zero
  if (width == 0 || height == 0) {
    int texture_width = 0;
    int texture_height = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    // Assign the dimesions as needed
    if (width == 0) {
      width = texture_width;
    }
    if (height == 0) {
      height = texture_height;
    }
  }

  ImGui::Image((void *)texture, ImVec2(width, height));
  return true;
}

bool displayTextureZoomable(SDL_Renderer *renderer, SDL_Texture *texture,
                            uint width, uint height, float previewNum,
                            float previewSize) {
  // Get dimensions of the texture only if either width or height is zero
  if (width == 0 || height == 0) {
    int texture_width = 0;
    int texture_height = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    // Assign the dimesions as needed
    if (width == 0) {
      width = texture_width;
    }
    if (height == 0) {
      height = texture_height;
    }
  }

  ImGui::ImageZoomable((void *)texture, ImVec2(width, height), previewNum,
                       previewSize);
  return true;
}

bool displayTextureZoomable(SDL_Renderer *renderer, SDL_Texture *texture,
                            uint width, uint height, uint dwidth, uint dheight,
                            float previewNum, float previewSize) {
  // Get dimensions of the texture only if either width or height is zero
  if (width == 0 || height == 0 || dwidth == 0 || dheight == 0) {
    int textureWidth = 0;
    int textureHeight = 0;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
    // Assign the dimesions as needed
    if (width == 0) {
      width = textureWidth;
    }
    if (height == 0) {
      height = textureHeight;
    }
    if (dwidth == 0) {
      dwidth = textureWidth;
    }
    if (dheight == 0) {
      dheight = textureHeight;
    }
  }
  ImGui::ImageZoomable((void *)texture, ImVec2(width, height),
                       ImVec2(dwidth, dheight), previewNum, previewSize);
  return true;
}

// For width and height, 0 indicates to use the respective dimension of the
// surface
bool displaySurface(SDL_Renderer *renderer, SDL_Surface *surface, uint width,
                    uint height) {
  if (surface == NULL) {
    return false;
  }

  SDL_Texture *texture_ptr = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture_ptr == NULL) {
    printf("Bad texture pointer");
    return false;
  }

  // Adjust width and height to the images if desired (width or height = 0)
  if (width == 0) {
    width = surface->w;
  }
  if (height == 0) {
    height = surface->h;
  }

  ImGui::Image((void *)texture_ptr, ImVec2(width, height));
  return true;
}

// Update the texture to whatever surface is.
// texture will always be destoryed, and can be NULL
// if surface is NULL, texture is returned
SDL_Texture *updateTexture(SDL_Renderer *renderer, SDL_Surface *surface,
                           SDL_Texture *texture) {
  if (surface == NULL) {
    return NULL;
  }

  if (texture != NULL) {
    // Memory cleanup
    SDL_DestroyTexture(texture);
    texture = NULL;
  }

  // Create the new texture to use
  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    fprintf(stderr, "updateTexture: Could not create texture from surface\n");
  }

  return texture;
}

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
                                              const Uint32 fmt) {
  if (src == NULL) {
    return NULL;
  }
  SDL_Surface *new_surface = SDL_ConvertSurfaceFormat(src, fmt, 0);
  SDL_FreeSurface(src);
  src = NULL;
  return new_surface;
}

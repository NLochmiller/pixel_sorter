#include <cmath>
#include <math.h>
#include <stdio.h>

#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "SDL_surface.h"
#include "global.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <SDL_image.h>

#include "imfilebrowser.h"

// Local includes
#include "bresenhamsLine_Interpolator.hpp"
#include "global.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error DearImGUI backend requires SDL 2.0.17+ because of SDL_RenderGeometry()
#endif

// Image types that are supported
#ifndef SUPPORTED_IMAGE_TYPES
#define SUPPORTED_IMAGE_TYPES                                                  \
  { ".png", ".jpg" }
#endif

// Forward declerations
void render(SDL_Renderer *renderer);
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer,
                SDL_Surface *&input_surface, SDL_Texture *&input_texture);

void handleMainMenuBar(ImGui::FileBrowser &inputFileDialog,
                       ImGui::FileBrowser &outputFileDialog) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::SeparatorText("Image files");
      // File dialogs to select files
      if (ImGui::MenuItem("Open", "", false)) {
        inputFileDialog.Open();
      }
      if (ImGui::MenuItem("Export as", "", false)) {
        outputFileDialog.Open();
      }
      ImGui::EndMenu();
    }
  }
  ImGui::EndMainMenuBar();
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

int main(int, char **) {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
    return -1;
  }

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with SDL_Renderer graphics context
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Window *window =
      SDL_CreateWindow("Pixel Sorter", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  if (window == nullptr) {
    fprintf(stderr, "Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return -1;
  }
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    SDL_Log("Error creating SDL_Renderer!");
    return 0;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // Enable Keyboard & gamepad controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup SDL2 renderer backend
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  // File interactions
  /* Input file dialog
   *   User should be able to select 1 file.
   *   User should be able to move through directories
   *   User should not be able to create files or directories
   *   User should only be able to select existing files
   *   Errors should not cause failures
   */
  ImGui::FileBrowser inputFileDialog(
      ImGuiFileBrowserFlags_SkipItemsCausingError);

  /* Export file dialog.
   *   User should be able to select 1 file
   *   User should be able to move through directories
   *   User should be able to create directories and 1 file
   *   Errors should not cause failures
   */
  ImGui::FileBrowser outputFileDialog(
      ImGuiFileBrowserFlags_EnterNewFilename |
      ImGuiFileBrowserFlags_CreateNewDir |
      ImGuiFileBrowserFlags_SkipItemsCausingError);
  inputFileDialog.SetTitle("Select input image");
  inputFileDialog.SetTypeFilters(SUPPORTED_IMAGE_TYPES);

  outputFileDialog.SetTitle("Select output image");
  outputFileDialog.SetTypeFilters(SUPPORTED_IMAGE_TYPES);

  /* Surfaces for images */
  SDL_Surface *input_surface = NULL;
  SDL_Surface *output_surface = NULL;

  /* Textures for images, used so we don't create one each frame */
  SDL_Texture *input_texture = NULL;
  SDL_Texture *output_texture = NULL;

  bool done = false;
  /* === START OF MAIN LOOP ================================================= */
  while (!done) {
    // Poll and handle events (inputs, window resize, etc.)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    main_window(viewport, renderer, input_surface, input_texture);
    handleMainMenuBar(inputFileDialog, outputFileDialog);

    // Process input file dialog
    inputFileDialog.Display();
    if (inputFileDialog.HasSelected()) {
      input_surface = IMG_Load(inputFileDialog.GetSelected().c_str());
      if (input_surface == NULL) {
        // TODO cancel file broser exit on error
        fprintf(stderr, "File %s does not exist\n",
                inputFileDialog.GetSelected().c_str());
      } else {
        input_texture = updateTexture(renderer, input_surface, input_texture);
        inputFileDialog.ClearSelected();
      }
    }

    // Process output file dialog
    outputFileDialog.Display();
    if (outputFileDialog.HasSelected()) {
      printf("Selected filename %s\n",
             outputFileDialog.GetSelected().string().c_str());
      outputFileDialog.ClearSelected();
    }

    render(renderer);
  }
  /* === END OF MAIN LOOP =================================================== */

  // Cleanup
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

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
bool displayTexture(SDL_Renderer *renderer, SDL_Texture *texture,
                    uint width = 0, uint height = 0) {
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

// For width and height, 0 indicates to use the respective dimension of the
// surface
bool displaySurface(SDL_Renderer *renderer, SDL_Surface *surface,
                    uint width = 0, uint height = 0) {
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

// The main window, aka the background window
// Returns non zero on error
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer,
                SDL_Surface *&input_surface, SDL_Texture *&input_texture) {
  static ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

  // Make fullscreen
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  if (ImGui::Begin("Main window", NULL, window_flags)) {
    // Main group
    ImGui::BeginGroup();
    static bool check = false;
    {
      ImGui::Checkbox("Test checkbox", &check);

      // Set the minimum and maximum percentages of values will be sorted
      static float min_percent = 25.0;
      static float max_percent = 75.0;
      ImGui::DragFloatRange2("Percentage range", &min_percent, &max_percent,
                             1.0f, 0.0f, 100.0f, "Minimum: %.2f%%",
                             "Maximum: %.2f%%", ImGuiSliderFlags_AlwaysClamp);
      ImGui::Text("min = %.3f max = %.3f", min_percent, max_percent);

      if (input_texture != NULL) {
        // Display image, for now at full size * max_percent
        double percent = max_percent / 100.0f;
        displayTexture(renderer, input_texture, input_surface->w * percent,
                       input_surface->h * percent);
      }
    }
    ImGui::EndGroup();

    // This is how to do a vertical layout, just split into 2 groups
    ImGui::SameLine();
    ImGui::BeginGroup();
    {
      ImGui::Text("First item on right");
      ImGui::Text("Second item on right");
    }
    ImGui::EndGroup();
  }
  ImGui::End();

  return 0;
}

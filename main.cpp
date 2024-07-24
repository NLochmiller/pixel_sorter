#include <stdio.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "SDL_surface.h"
#include "global.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

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
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer);

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
    main_window(viewport, renderer);
    handleMainMenuBar(inputFileDialog, outputFileDialog);

    // Process input file dialog
    inputFileDialog.Display();
    if (inputFileDialog.HasSelected()) {
      printf("Selected filename %s\n",
             inputFileDialog.GetSelected().string().c_str());
      inputFileDialog.ClearSelected();
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
    exit(-2);
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

bool test_for_mac(int &a) {
  a = 30;
  return a < 45;
}

void test_octant(int &currentX, int &currentY, int sx, int sy, int ex, int ey,
                 int &dx, int &dy, double &slope_error,
                 SDL_Surface *img_surface, double percent) {
  // For each octant
  LineInterpolator::init_bresenhams(currentX, currentY, sx, sy, ex, ey, dx, dy,
                                    slope_error);

  bresenham_interpolator* func = LineInterpolator::get_interpolator(0);

  printf("(%d, %d) to (%d, %d)\n", sx, sy, ex, ey);
  Uint32 black = SDL_MapRGBA(img_surface->format, (Uint8)255 * percent, 0, 0, 255);
  Uint32 *pixels = (Uint32 *)img_surface->pixels;
  do {
    printf("  (%d, %d)\n", currentX, currentY);
    // n*WIDTH+m
    pixels[currentY * img_surface->h + currentX] = black;
  } while (func(currentX, currentY, sx, sy, ex, ey, dx, dy, slope_error));
}

void test(SDL_Renderer *renderer) {
  static bool init = false;
  int w = 101;
  int h = 101;
  static SDL_Surface *img_surface =
      SDL_CreateRGBSurfaceWithFormat(0, w, h, 8, DEFAULT_PIXEL_FORMAT);
  // IMG_Load("/home/nloch/Pictures/backgrounds/wallhaven-q6ro3l.jpg");

  if (!init) {
    if (img_surface == NULL) {
      printf("BAD SURFACE\n");
      exit(-1);
    }

    // Fill with white
    const SDL_Rect whole_surf_rect = {.x = 0, .y = 0, .w = w, .h = h};
    Uint32 background_color =
        SDL_MapRGBA(img_surface->format, 255, 255, 255, 255);

    SDL_FillRect(img_surface, &whole_surf_rect, (Uint32)background_color);

    // Number of segments to test with
    double segments = 8.0f;
    double dang = 360.0f / segments;
    int hyp_len = 20;

    for (double ang = 0; ang < 360.0f; ang += dang) {
      printf("angle %f\n", ang);
      double ang_in_rads = ang * (M_PI/180.0f);
      int curX, curY;
      int sx = w/2;
      int sy = h/2;
      int ex = sx + cos(-ang_in_rads) * hyp_len;
      int ey = sy + sin(-ang_in_rads) * hyp_len;
      int dx = 0;
      int dy = 0;
      double slope_error = 0;

      test_octant(curX, curY, sx, sy, ex, ey, dx, dy, slope_error, img_surface, ang/360.0f);
    }

    init = true;
  }

  h = 300;
  displaySurface(renderer, img_surface, h, h);
}

// The main window, aka the background window
// Returns non zero on error
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer) {
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
      test(renderer);
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

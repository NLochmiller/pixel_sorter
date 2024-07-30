#include <cmath>
#include <cstdint>
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
#include <utility>

#include "imfilebrowser.h"

// Local includes
#include "ImGui_SDL2_helpers.hpp"
#include "LineInterpolator.hpp"
#include "PixelSorter.hpp"
#include "global.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error DearImGUI backend requires SDL 2.0.17+ because of SDL_RenderGeometry()
#endif

const uint32_t DEFAULT_PIXEL_FORMAT = SDL_PIXELFORMAT_ABGR8888;

// TODO: MOVE TO BETTER LOCATION

typedef std::pair<double, double> point;

// Return where two lines given points A and B intersect
point lineLineIntersection(point A, point B, point C, point D) {
  // Line AB represented as a1x + b1y = c1
  double a1 = B.second - A.second;
  double b1 = A.first - B.first;
  double c1 = a1 * (A.first) + b1 * (A.second);

  // Line CD represented as a2x + b2y = c2
  double a2 = D.second - C.second;
  double b2 = C.first - D.first;
  double c2 = a2 * (C.first) + b2 * (C.second);

  double determinant = a1 * b2 - a2 * b1;

  if (determinant == 0) {
    // The lines are parallel. This is simplified
    // by returning a pair of FLT_MAX
    return std::make_pair(FLT_MAX, FLT_MAX);
  } else {
    double x = (b2 * c1 - b1 * c2) / determinant;
    double y = (a1 * c2 - a2 * c1) / determinant;
    return std::make_pair(x, y);
  }
}

// Return the end point normalized (as if we start at 0,0)
point getEndPoint(double angle, double angle_degrees, double width,
                  double height) {
  point topRight = std::make_pair(width, height);
  point topLeft = std::make_pair(-width, height);
  point botRight = std::make_pair(width, -height);
  point botLeft = std::make_pair(-width, -height);

  point origin = std::make_pair(0, 0);
  int length = 10; // just a test length
  point lineEnd =
      std::make_pair(length * std::cos(angle), length * std::sin(angle));

  // Check intersection with x's
  point posX = lineLineIntersection(origin, lineEnd, botRight, topRight);
  point negX = lineLineIntersection(origin, lineEnd, botLeft, topLeft);

  // Check intersection with y's
  point posY = lineLineIntersection(origin, lineEnd, topLeft, topRight);
  point negY = lineLineIntersection(origin, lineEnd, botLeft, botRight);
#define IS_PARALLEL(_p_) (_p_.first == FLT_MAX && _p_.second == FLT_MAX)

  if (angle_degrees == 0 || angle_degrees == 360) {
    return std::make_pair(width, 0); // Right +x, 0y
  } else if (angle_degrees == 90) {
    return std::make_pair(0, height); // Up 0x, +y
  } else if (angle_degrees == 180) {
    return std::make_pair(-width, 0); // Left -x, 0y
  } else if (angle_degrees == 270) {
    return std::make_pair(0, -height); // Down 0x, -y
  } else if (angle_degrees >= 0 && angle_degrees < 90) {
    // top right, Only care about posX and posY
    if (IS_PARALLEL(posX) || posX.first > width || posX.second > height) {
      return posY; // because posX is out of bounds or line is parralel to X
    } else if (IS_PARALLEL(posY) || posY.first > width ||
               posY.second > height) {
      return posX;
    }
  } else if (angle_degrees >= 90 && angle_degrees < 180) {
    // top left, Only care about negX and posY
    if (IS_PARALLEL(negX) || negX.first < -width || negX.second > height) {
      return posY; // because negX is out of bounds or line is parralel to X
    } else if (IS_PARALLEL(posY) || posY.first < -width ||
               posY.second > height) {
      return negX; // because negY is out of bounds or line is parralel to Y
    }
  } else if (angle_degrees >= 180 && angle_degrees < 270) {
    // Bottom left only care about negX and negY
    if (IS_PARALLEL(negX) || negX.first < -width || negX.second < -height) {
      return negY; // because negX is out of bounds or line is parralel to X
    } else if (IS_PARALLEL(negY) || negY.first < -width ||
               negY.second < -height) {
      return negX; // because negY is out of bounds or line is parralel to Y
    }
  } else if (angle_degrees >= 270 && angle_degrees < 360) {
    // Bottom right only care about posX and negY
    if (IS_PARALLEL(posX) || posX.first > width || posX.second < -height) {
      return negY; // because posX is out of bounds or line is parralel to X
    } else if (IS_PARALLEL(negY) || negY.first > width ||
               negY.second < -height) {
      return posX; // because negY is out of bounds or line is parralel to Y
    }
  }

  return std::make_pair(0, 0);
}

// Wrapper for the PixelSorter::sort function, converts surfaces to pixel arrays
// to pass onto it, and assembles some needed information
bool sort_wrapper(SDL_Renderer *renderer, SDL_Surface *&input_surface,
                  SDL_Surface *&output_surface, double angle) {
  if (input_surface == NULL || output_surface == NULL) {
    return false;
  }

  // Test code that fills output with white
  {
    const SDL_Rect whole_surf_rect = {
        .x = 0, .y = 0, .w = output_surface->w, .h = output_surface->h};
    Uint32 background_color =
        SDL_MapRGBA(output_surface->format, 255, 255, 255, 255);

    SDL_FillRect(output_surface, &whole_surf_rect, (Uint32)background_color);
  }

  // While I would rather cast and pass directly, must do this so that the
  // compiler will stop complaining
  PixelSorter_Pixel_t *input_pixels = (uint32_t *)input_surface->pixels;
  PixelSorter_Pixel_t *output_pixels = (uint32_t *)output_surface->pixels;

  // Generate the line
  if (angle == 360) {
    angle = 0;
  }
  int currentX = 0, currentY = 0, startX = 0, startY = 0, endX = 0, endY = 0,
      deltaX = 0, deltaY = 0;
  double slope_error;

  double ang_in_rads = angle * (M_PI / 180.0f);
  point endPoint =
      getEndPoint(ang_in_rads, angle, input_surface->w, input_surface->h);
  deltaX = (int)std::round(endPoint.first);
  deltaY = (int)std::round(endPoint.second);
  if (angle >= 0 && angle < 90) { // +x +y quadrant
    startX = 0;
    startY = 0;
  } else if (angle >= 90 && angle < 180) { // -x +y quadrant
    startX = input_surface->w - 1;
    startY = 0;
  } else if (angle >= 180 && angle < 270) { // -x -y quadrant
    startX = input_surface->w - 1;
    startY = input_surface->h - 1;
  } else { // +x -y quadrant
    startX = 0;
    startY = input_surface->h - 1;
  }
  endX = deltaX + startX;
  endY = deltaY + startY;

  LineInterpolator::init_bresenhams(currentX, currentY, startX, startY, endX,
                                    endY, deltaX, deltaY, slope_error);

  // Add each point to a queue
  bresenham_interpolator *interpolator =
      LineInterpolator::get_interpolator(deltaX, deltaY);

  

  do {
    if (0 <= currentX && currentX < output_surface->w && 0 <= currentY &&
        currentY < output_surface->h) {

      output_pixels[TWOD_TO_1D(currentX, currentY, output_surface->w)] =
          SDL_MapRGB(input_surface->format, 255, 0, 0);
      // printf("(%d, %d)\n", currentX, currentY);
      // Add point to a queue
    }
  } while (interpolator(currentX, currentY, endX, endY, deltaX, deltaY,
                        slope_error));

  /* LINE GENERATION DONE */

  /*
   * Now in the perspective of 1 dimension L where L is the dimension with
   * greater change, that is:
   *   if |deltaX| >= |deltaY|, L = X.
   *   if |deltaX| <  |deltaY|, L - Y.
   *
   * There is also another dimension S where S is the opposite of L, that is:
   *   if L = X, S = Y.
   *   If L = Y, S = X
   */
  /* Bad code that would affect how line works
  // Use pointers to save on lines of code
  int *startL = NULL;
  int *endL = NULL;
  int *deltaL = NULL;
  int *deltaS = NULL;

  // Change deltaL by deltaS where S is the dimension with a smaller delta
  // without changing where the center of the N lines are
  if (std::abs(deltaX) >= std::abs(deltaY)) { // X changes more than Y or same
    startL = &startX;
    endL = &endX;
    deltaL = &deltaX;
    deltaS = &deltaY;
  } else { // Y changes more than X
    startL = &startY;
    endL = &endY;
    deltaL = &deltaY;
    deltaS = &deltaX;
  }
  // Extend start and end by offset, causing the deltaL to grow by 2*deltaS
  int offset = ((*deltaL >= 0) ? *deltaS : -*deltaS);
  (*startL) += offset;
  (*endL) -= offset;
  */

  // For each n starting from startN to endN, sort along that line

  PixelSorter::sort(input_pixels, output_pixels);
  return true;
}

// Forward declerations
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer,
                SDL_Surface *&input_surface, SDL_Texture *&input_texture,
                SDL_Surface *&output_surface, SDL_Texture *&output_texture,
                std::filesystem::path *output_path);

void handleMainMenuBar(ImGui::FileBrowser &inputFileDialog,
                       ImGui::FileBrowser &outputFileDialog);

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
  std::filesystem::path output_path;
  SDL_Texture *input_texture = NULL;
  SDL_Texture *output_texture = NULL;

  // TODO: REMOVE_START
  // This is just to get the compiler to be quiet
  fprintf(stderr, "quiet compiler %p %p\n", output_surface, output_texture);
  // TODO: REMOVE_END,

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
    main_window(viewport, renderer, input_surface, input_texture,
                output_surface, output_texture, NULL);
    handleMainMenuBar(inputFileDialog, outputFileDialog);

    // Process input file dialog
    inputFileDialog.Display();
    if (inputFileDialog.HasSelected()) {
      // input_surface = IMG_Load(inputFileDialog.GetSelected().c_str());
      input_surface =
          SDL_CreateRGBSurfaceWithFormat(0, 100, 100, 8, DEFAULT_PIXEL_FORMAT);

      if (input_surface == NULL) {
        // TODO cancel file broser exit on error
        fprintf(stderr, "File %s does not exist\n",
                inputFileDialog.GetSelected().c_str());
      } else {
        // Immediately convert to the basic format
        input_surface = SDL_ConvertSurfaceFormat_MemSafe(input_surface,
                                                         DEFAULT_PIXEL_FORMAT);

        // Convert to texture
        input_texture = updateTexture(renderer, input_surface, input_texture);

        // Create the output surface to use with this
        output_surface = SDL_CreateRGBSurfaceWithFormat(
            0, input_surface->w, input_surface->h, DEFAULT_DEPTH,
            DEFAULT_PIXEL_FORMAT);

        if (output_surface == NULL) {
          fprintf(stderr, "Failed to create output surface");
        } else {
          output_texture =
              updateTexture(renderer, output_surface, output_texture);
        }
        inputFileDialog.ClearSelected();
      }
    }

    // Process output file dialog
    outputFileDialog.Display();
    if (outputFileDialog.HasSelected()) {
      output_path = outputFileDialog.GetSelected();
      printf("Selected filename %s\n", output_path.c_str());

      // TODO: Save output_surface to a image

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

// Handle the main menu bar
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

// The main window, aka the background window
// Returns non zero on error
int main_window(const ImGuiViewport *viewport, SDL_Renderer *renderer,
                SDL_Surface *&input_surface, SDL_Texture *&input_texture,
                SDL_Surface *&output_surface, SDL_Texture *&output_texture,
                std::filesystem::path *output_path) {
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

      static float angle = 90.0;
      ImGui::DragFloat("Sort angle", &angle, 1.0f, 0.0f, 360.0f, "%.2f");

      // Export button
      {
        // TODO: Find if path is empty
        bool is_export_button_disabled =
            output_surface == NULL /* || PATH BAD */;
        ImGui::BeginDisabled(is_export_button_disabled);
        if (ImGui::Button("Export")) {
          fprintf(stderr, "export!\n");
          // TODO: Save to export path.
        }

        ImGui::EndDisabled();
      }

      // Start sorting
      if (ImGui::Button("Sort")) {
        sort_wrapper(renderer, input_surface, output_surface, angle);
        output_texture =
            updateTexture(renderer, output_surface, output_texture);
      }

      // Zoom slider
      static float image_zoom = 100.0;
      ImGui::DragFloat("Image zoom", &image_zoom, 1.0f, 0.0f, 500.0f,
                       "Zoom: %.2f%%", 0);

      // Display images
      double zoom_percent = image_zoom / 100.0f;
      // Display input image zoomed in to percent
      if (input_texture != NULL) {
        // TODO: Renable this
        // displayTexture(renderer, input_texture, input_surface->w *
        // zoom_percent,
        //                input_surface->h * zoom_percent);
      }

      // Display output image zoomed in to percent
      if (output_texture != NULL) {
        displayTexture(renderer, output_texture,
                       output_surface->w * zoom_percent,
                       output_surface->h * zoom_percent);
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

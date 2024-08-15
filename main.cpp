#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <math.h>
#include <queue>
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

typedef std::pair<double, double> point_doubles;
using point_ints = std::pair<int, int>;
using pointQueue = std::queue<point_ints>;

// Return the intersection of the line from x,y to the center of min[XY] max[XY]
point_doubles pointOnRect(double x, double y, double minX, double maxX,
                          double minY, double maxY) {
  double midX = (minX + maxX) / 2;
  double midY = (minY + maxY) / 2;
  double m = (midY - y) / (midX - x);

  if (x <= midX) { // check "left" side
    double minXy = m * (minX - x) + y;
    if (minY <= minXy && minXy <= maxY)
      return std::make_pair(minX, minXy);
  }

  if (x >= midX) { // check "right" side
    double maxXy = m * (maxX - x) + y;
    if (minY <= maxXy && maxXy <= maxY)
      return std::make_pair(maxX, maxXy);
  }

  if (y <= midY) { // check "top" side
    double minYx = (minY - y) / m + x;
    if (minX <= minYx && minYx <= maxX)
      return std::make_pair(minYx, minY);
  }

  if (y >= midY) { // check "bottom" side
    double maxYx = (maxY - y) / m + x;
    if (minX <= maxYx && maxYx <= maxX)
      return std::make_pair(maxYx, maxY);
  }
  // edge case when finding midpoint intersection: m = 0/0 = NaN
  if (x == midX && y == midY) {
    return std::make_pair(0.0, 0.0);
  }

  fprintf(stderr, "pointOnRect: UNACCOUNTED CASE\n"); // Error print.
  return std::make_pair(0.0, 0.0);
}

// Return the end point normalized (as if we start at 0,0)
point_doubles getEndPoint(double angle, double angle_degrees, double width,
                          double height) {
  // Arbitrary number, essentially controls precision of the end point
  double length = width * width + height * height;
  point_doubles smallLine =
      std::make_pair(length * std::cos(angle), length * std::sin(angle));

  // maximum dimension
  double maxD = std::abs((std::abs(width) > std::abs(height)) ? width : height);
  // Find the point on rect that is centered on origin, where a line can be
  // drawn to it from origin with given angle from 0 degrees
  return pointOnRect(smallLine.first, smallLine.second, -maxD, maxD, -maxD,
                     maxD);
}

// Generate a Bresenham's line at angle that goes from origin to any edge of the
// rectangle. With the origin being (0, 0), the line starts at the origin, and
// the rectangle is centered on the origin
pointQueue generateLinePointQueueFitIntoRectangle(double &angle, int halfwidth,
                                                  int halfheight,
                                                  BresenhamsArguments &args) {
  // Generate the line
  if (angle == 360) {
    angle = 0;
  }
  // Calculate end point
  double ang_in_rads = angle * (M_PI / 180.0f);
  point_doubles endPoint =
      getEndPoint(ang_in_rads, angle, halfwidth, halfheight);

  // Initalize arguments to go from (0,0) to calculated end point
  args.init(0, 0, (int)std::round(endPoint.first),
            (int)std::round(endPoint.second));
  bresenham_interpolator *interpolator =
      LineInterpolator::get_interpolator(args.deltaX, args.deltaY);
  // Add each point to output queue
  std::queue<std::pair<int, int>> points;
  do {
    points.push(std::make_pair(args.currentX, args.currentY));
  } while (interpolator(args));
  return points;
}

// Wrapper for the PixelSorter::sort function, converts surfaces to pixel
// arrays to pass onto it, and assembles some needed information
bool sort_wrapper(SDL_Renderer *renderer, SDL_Surface *&input_surface,
                  SDL_Surface *&output_surface, double angle, double valueMin,
                  double valueMax) {
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
  BresenhamsArguments bresenhamsArgs(0, 0);
  pointQueue pointQueue = generateLinePointQueueFitIntoRectangle(
      angle, input_surface->w, input_surface->h, bresenhamsArgs);
  int numPoints = pointQueue.size();

  // Convert point queue to array of points
  point_ints *points =
      (point_ints *)calloc(sizeof(point_ints), pointQueue.size());
  if (points == NULL) {
    fprintf(stderr, "Unable to convert point queue to array\n");
    return false;
  }
  for (int i = 0; i < numPoints && !pointQueue.empty(); i++) {
    points[i] = pointQueue.front();
    pointQueue.pop();
  }

  // Start and end coordinates for making multiple lines
  int startX = 0;
  int startY = 0;
  int endX = 0;
  int endY = 0;

  // Shift to specific corner for each quadrant
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
  // Properly set endX and endY
  endX = bresenhamsArgs.deltaX + startX;
  endY = bresenhamsArgs.deltaY + startY;

  PixelSorter::sort(input_pixels, output_pixels, points, numPoints,
                    input_surface->w, input_surface->h, startX, startY, endX,
                    endY, valueMin / 100, valueMax / 100, input_surface);
  free(points);
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

  bool done = false;
  /* === START OF MAIN LOOP =================================================
   */
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
      input_surface = IMG_Load(inputFileDialog.GetSelected().c_str());
      // TODO: Remove test code
      // input_surface =
      //     SDL_CreateRGBSurfaceWithFormat(0, 100, 100, 8,
      //     DEFAULT_PIXEL_FORMAT);

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
  /* === END OF MAIN LOOP ===================================================
   */

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
        // Angle input is human readable, account for screen 0,0 being top left
        double flippedAngle = 360 - angle;
        sort_wrapper(renderer, input_surface, output_surface, flippedAngle,
                     min_percent, max_percent);
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
        displayTexture(renderer, input_texture, input_surface->w * zoom_percent,
                       input_surface->h * zoom_percent);
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

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <stdio.h>

#include "SDL_pixels.h"
#include "SDL_render.h"
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
  double angInRads = angle * (M_PI / 180.0f);
  point_doubles endPoint = getEndPoint(angInRads, angle, halfwidth, halfheight);

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
bool sort_wrapper(SDL_Renderer *renderer, SDL_Surface *&inputSurface,
                  SDL_Surface *&outputSurface, double angle, double valueMin,
                  double valueMax) {
  if (inputSurface == NULL || outputSurface == NULL) {
    return false;
  }
  // While I would rather cast and pass directly, must do this so that the
  // compiler will stop complaining
  PixelSorter_Pixel_t *inputPixels = (uint32_t *)inputSurface->pixels;
  PixelSorter_Pixel_t *outputPixels = (uint32_t *)outputSurface->pixels;
  // Generate the line
  BresenhamsArguments bresenhamsArgs(0, 0);
  pointQueue pointQueue = generateLinePointQueueFitIntoRectangle(
      angle, inputSurface->w, inputSurface->h, bresenhamsArgs);
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
    startX = inputSurface->w - 1;
    startY = 0;
  } else if (angle >= 180 && angle < 270) { // -x -y quadrant
    startX = inputSurface->w - 1;
    startY = inputSurface->h - 1;
  } else { // +x -y quadrant
    startX = 0;
    startY = inputSurface->h - 1;
  }
  // Properly set endX and endY
  endX = bresenhamsArgs.deltaX + startX;
  endY = bresenhamsArgs.deltaY + startY;

  PixelSorter::sort(inputPixels, outputPixels, points, numPoints,
                    inputSurface->w, inputSurface->h, startX, startY, endX,
                    endY, valueMin / 100, valueMax / 100, inputSurface);
  free(points);
  return true;
}

// Forward declerations
int mainWindow(const ImGuiViewport *viewport, SDL_Renderer *renderer,
               SDL_Surface *&inputSurface, SDL_Texture *&inputTexture,
               SDL_Surface *&outputSurface, SDL_Texture *&outputTexture,
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
  SDL_Surface *inputSurface = NULL;
  SDL_Surface *outputSurface = NULL;

  /* Textures for images, used so we don't create one each frame */
  std::filesystem::path outputPath;
  SDL_Texture *inputTexture = NULL;
  SDL_Texture *outputTexture = NULL;

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
    mainWindow(viewport, renderer, inputSurface, inputTexture, outputSurface,
               outputTexture, NULL);
    handleMainMenuBar(inputFileDialog, outputFileDialog);

    // Process input file dialog
    inputFileDialog.Display();
    if (inputFileDialog.HasSelected()) {
      inputSurface = IMG_Load(inputFileDialog.GetSelected().c_str());
      if (inputSurface == NULL) {
        // TODO cancel file broser exit on error
        fprintf(stderr, "File %s does not exist\n",
                inputFileDialog.GetSelected().c_str());
      } else {
        // Immediately convert to the basic format
        inputSurface = SDL_ConvertSurfaceFormat_MemSafe(inputSurface,
                                                        DEFAULT_PIXEL_FORMAT);
        // Convert to texture
        inputTexture = updateTexture(renderer, inputSurface, inputTexture);
        // Create the output surface to use with this
        outputSurface =
            SDL_CreateRGBSurfaceWithFormat(0, inputSurface->w, inputSurface->h,
                                           DEFAULT_DEPTH, DEFAULT_PIXEL_FORMAT);
        if (outputSurface == NULL) {
          fprintf(stderr, "Failed to create output surface");
        } else {
          outputTexture = updateTexture(renderer, outputSurface, outputTexture);
        }
        inputFileDialog.ClearSelected();
      }
    }

    // Process output file dialog
    outputFileDialog.Display();
    if (outputFileDialog.HasSelected()) {
      outputPath = outputFileDialog.GetSelected();
      printf("Selected filename %s\n", outputPath.c_str());
      // TODO: MOVE TO A SAVE IMAGE FUNCTION
      if (outputSurface != NULL) {
        // TODO: Allow selection between .png and .jpg formating
        IMG_SavePNG(outputSurface, outputPath.c_str());
      } else {
        fprintf(stderr, "The output image does not exist! You must sort before "
                        "exporting!\n");
      }
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
int mainWindow(const ImGuiViewport *viewport, SDL_Renderer *renderer,
               SDL_Surface *&inputSurface, SDL_Texture *&inputTexture,
               SDL_Surface *&outputSurface, SDL_Texture *&outputTexture,
               std::filesystem::path *outputPath) {
  static ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

  // Make fullscreen
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  if (ImGui::Begin("Main window", NULL, windowFlags)) {
    // Main group
    ImGui::BeginGroup();
    static bool check = false;
    {
      ImGui::Checkbox("Test checkbox", &check);

      // Set the minimum and maximum percentages of values will be sorted
      static float percentMin = 25.0;
      static float percentMax = 75.0;
      ImGui::DragFloatRange2("Percentage range", &percentMin, &percentMax, 1.0f,
                             0.0f, 100.0f, "Minimum: %.2f%%", "Maximum: %.2f%%",
                             ImGuiSliderFlags_AlwaysClamp);
      ImGui::Text("min = %.3f max = %.3f", percentMin, percentMax);

      static float angle = 90.0;
      ImGui::DragFloat("Sort angle", &angle, 1.0f, 0.0f, 360.0f, "%.2f");

      // Export button
      {
        // TODO: Find if path is empty
        bool isExportButtonDisabled =
            (outputSurface == NULL) /* TODO: || PATH BAD */;
        ImGui::BeginDisabled(isExportButtonDisabled);
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
        sort_wrapper(renderer, inputSurface, outputSurface, flippedAngle,
                     percentMin, percentMax);
        outputTexture = updateTexture(renderer, outputSurface, outputTexture);
      }

      // Zoom slider
      static float imageZoom = 100.0;
      ImGui::DragFloat("Image zoom", &imageZoom, 1.0f, 0.0f, 500.0f,
                       "Zoom: %.2f%%", 0);

      // Display images
      double zoomPercent = imageZoom / 100.0f;
      // Display input image zoomed in to percent
      if (inputTexture != NULL) {
        displayTexture(renderer, inputTexture, inputSurface->w * zoomPercent,
                       inputSurface->h * zoomPercent);
      }

      // Display output image zoomed in to percent
      if (outputTexture != NULL) {
        displayTexture(renderer, outputTexture, outputSurface->w * zoomPercent,
                       outputSurface->h * zoomPercent);
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>

#include "Knob.hpp"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_surface.h"

// Enable math operators for imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <SDL_image.h>
#include <utility>

#include "imfilebrowser.h"

// Local includes
#include "ColorConversion.hpp"
#include "ImGui_SDL2_helpers.hpp"
#include "LineCollision.hpp"
#include "LineInterpolator.hpp"
#include "PixelSorter.hpp"
#include "global.hpp"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error DearImGUI backend requires SDL 2.0.17+ because of SDL_RenderGeometry()
#endif

using LineCollision::pointQueue;

// Definition of constants
const uint32_t DEFAULT_PIXEL_FORMAT = SDL_PIXELFORMAT_ABGR8888;

// Scale source such that it takes up the most space it can within bounds.
ImVec2 maximizeImVec2WithinBounds(const ImVec2 &source, const ImVec2 &bounds) {
  // Error check
  if (source.x <= 0 || source.y <= 0) {
    return ImVec2(0, 0);
  }
  // Calculate scaling factors for both images
  float width_scale = bounds.x / source.x;
  float height_scale = bounds.y / source.y;
  float scale = std::min(width_scale, height_scale);

  // Calculate scaled dimensions for both images (they will be the same)
  return ImVec2(scale * source.x, scale * source.y);
}

// Display the images, for now layout where input above output
void displayTiledZoomableImages(
    const ImGuiViewport *viewport, SDL_Renderer *renderer,
    SDL_Surface *&inputSurface, SDL_Texture *&inputTexture,
    SDL_Surface *&outputSurface, SDL_Texture *&outputTexture,
    float &minDimension, float &previewNum, float &previewSize) {

  static ImVec2 childSize = ImVec2(0, 0);
  ImGuiChildFlags childFlags = 0;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration;

  /* Calculate which layout to use, and display image size */
  if (ImGui::BeginChild("Image display child", ImVec2(0, 0), childFlags,
                        windowFlags)) {
    childSize = ImGui::GetWindowSize();
    ImVec2 display = ImVec2(0, 0);
    // Calculate the maximum size that each image is allowed to take up
    bool useHoriLayout = true; // Should we use the horizontal layout?
    ImGuiStyle &style = ImGui::GetStyle();
    // Maximum area that both images are in, accounting for padding
    ImVec2 max_images_area = ImVec2(childSize.x, childSize.y);
    max_images_area = max_images_area - style.WindowPadding;

    // Display input image zoomed in to percent
    if (inputTexture != NULL) {
      // We have an image, display it
      ImVec2 input_image_scale = ImVec2(inputSurface->w, inputSurface->h);
      // display = clampImVec2ToBounds(input_image_scale, max_image_scale);
      // Store the original area of max_images_area to restore later
      ImVec2 original_size = max_images_area;

      /* Calculate size for the horizontal layout */
      // Account for spacing, then divide horizontally to find spacing for
      // the horizontal layout
      max_images_area.x = (max_images_area.x - style.ItemSpacing.x) / 2;
      ImVec2 display_h =
          maximizeImVec2WithinBounds(input_image_scale, max_images_area);
      max_images_area = original_size;

      /* Calculate size for the vertical layout */
      // Account for spacing, then divide horizontally to find spacing for
      // the horizontal layout
      max_images_area.y = (max_images_area.y - style.ItemSpacing.y) / 2;
      ImVec2 display_v =
          maximizeImVec2WithinBounds(input_image_scale, max_images_area);
      max_images_area = original_size;

      // Find the maximum area either image can occupy
      if (display_h.x >= display_v.x) {
        useHoriLayout = true; // Use horizontal layout
        display = display_h;
      } else {
        useHoriLayout = false; // Use vertical layoyt
        display = display_v;
      }
    }

    /* Display images */
    if (inputSurface != NULL) {
      displayTextureZoomable(renderer, inputTexture, inputSurface->w,
                             inputSurface->h, display.x, display.y, previewNum,
                             previewSize);
    }

    // Display vertical aspect images on the same line
    if (useHoriLayout) {
      ImGui::SameLine();
    }

    // Display output image zoomed in to percent
    if (outputTexture != NULL) {
      displayTextureZoomable(renderer, outputTexture, outputSurface->w,
                             outputSurface->h, display.x, display.y, previewNum,
                             previewSize);
    }
  }
  ImGui::EndChild();
}

// Wrapper for the PixelSorter::sort function, converts surfaces to pixel
// arrays to pass onto it, and assembles some needed information
bool sort_wrapper(SDL_Renderer *renderer, SDL_Surface *&inputSurface,
                  SDL_Surface *&outputSurface, double angle, double valueMin,
                  double valueMax, ColorConverter *converter) {
  if (inputSurface == NULL || outputSurface == NULL) {
    return false;
  }
  // While I would rather cast and pass directly, must do this so that the
  // compiler will stop complaining
  PixelSorter_Pixel_t *inputPixels = (uint32_t *)inputSurface->pixels;
  PixelSorter_Pixel_t *outputPixels = (uint32_t *)outputSurface->pixels;
  // Generate the line
  BresenhamsArguments bresenhamsArgs(0, 0);
  pointQueue pointQueue = LineCollision::generateLineQueueForRect(
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
                    endY, valueMin / 100, valueMax / 100, converter,
                    inputSurface->format);
  free(points);
  return true;
}

// Forward declerations
int mainWindow(const ImGuiViewport *viewport, SDL_Renderer *renderer,
               SDL_Surface *&inputSurface, SDL_Texture *&inputTexture,
               SDL_Surface *&outputSurface, SDL_Texture *&outputTexture,
               std::filesystem::path *output_path, ColorConverter **converter);

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

  ColorConverter *converter = &(ColorConversion::average);

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
               outputTexture, NULL, &converter);
    handleMainMenuBar(inputFileDialog, outputFileDialog);

    // Process input file dialog
    inputFileDialog.Display();
    if (inputFileDialog.HasSelected()) {
      inputSurface = IMG_Load(inputFileDialog.GetSelected().c_str());
      if (inputSurface == NULL) {
        // TODO cancel file browser exit on error
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
    // TODO BEGIN REMOVE
    bool debug_show_style = false;
    if (debug_show_style) {
      ImGui::Begin("style");
      ImGui::ShowStyleEditor(NULL);
      ImGui::End();
    }
    // TODO END REMOVE

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
               std::filesystem::path *outputPath, ColorConverter **converter) {
  static ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

  // Make fullscreen
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  if (ImGui::Begin("Main window", NULL, windowFlags)) {
    { // Color Conversion selection
      // The pairs that make up the selection options
      static std::pair<ColorConverter *, char *> converterOptions[] = {
          std::make_pair(&(ColorConversion::red), (char *)"Red"),
          std::make_pair(&ColorConversion::green, (char *)"Green"),
          std::make_pair(&ColorConversion::blue, (char *)"Blue"),
          std::make_pair(&ColorConversion::average, (char *)"Average"),
          std::make_pair(&ColorConversion::minimum, (char *)"Minimum"),
          std::make_pair(&ColorConversion::maximum, (char *)"Maximum"),
          std::make_pair(&ColorConversion::chroma, (char *)"Chroma"),
          std::make_pair(&ColorConversion::hue, (char *)"Hue"),
          std::make_pair(&ColorConversion::saturation,
                         (char *)"Saturation (HSV)"),
          std::make_pair(&ColorConversion::value, (char *)"Value"),
          std::make_pair(&ColorConversion::saturation_HSL,
                         (char *)"Saturation (HSL)"),
          std::make_pair(&ColorConversion::lightness, (char *)"Lightness")};
      static const int convertersCount = arrayLen(converterOptions);
      static int selected_converter_index = 11; // Use lightness as default
      // Pass in the preview value visible before opening the combo (it could
      // technically be different contents or not pulled from items[])
      const char *combo_preview_value =
          converterOptions[selected_converter_index].second;
      static ImGuiComboFlags flags = 0;
      // Display each item in combo
      if (ImGui::BeginCombo("Pixel Quantizer", combo_preview_value, flags)) {
        for (int n = 0; n < convertersCount; n++) {
          const bool is_selected = (selected_converter_index == n);
          if (ImGui::Selectable(converterOptions[n].second, is_selected))
            selected_converter_index = n;

          // Set the initial focus when opening the combo (scrolling +
          // keyboard navigation focus)
          if (is_selected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      *converter = converterOptions[selected_converter_index].first; // update
    }

    const ImGuiSliderFlags sliderFlags = ImGuiSliderFlags_AlwaysClamp;

    // Set the minimum and maximum percentages of values will be sorted
    static float percentMin = 25.0;
    static float percentMax = 75.0;
    ImGui::DragFloatRange2("Percentage range", &percentMin, &percentMax, 1.0f,
                           0.0f, 100.0f, "Minimum: %.2f%%", "Maximum: %.2f%%",
                           sliderFlags);

    static float angle = 0;
    const static float low = 0.0f;
    const static float high_angle = 360;
    const static float high_radian = 2 * M_PI;
    float displayAngle =
        std::clamp((float)(360 - (angle * 180 / M_PI)), low, high_angle);

    if (ImGui::DragFloat("Sort angle", &displayAngle, 1.0f, 0.0f, 360.0f,
                         "%.2f", sliderFlags)) {
      // There has been a change. Change the angle to repersent this change
      // in the display angle
      angle = (360 - displayAngle) * M_PI / 180;
      std::clamp(angle, low, high_radian);
    }
    float knob_angle = angle;
    if (ImGui::Knob("Sort angle knob", &knob_angle, 50.0f)) {
      // Knob has caused a change, update the angle
      angle = knob_angle;
      std::clamp(angle, low, high_radian);
    }

    { // Sorting button. Enabled only when there is an input surface
      ImGui::BeginDisabled(inputSurface == NULL);
      if (ImGui::Button("Sort")) {
        // Angle input is human readable, account for screen 0,0 being top
        // left
        double flippedAngle = 360 - angle;
        sort_wrapper(renderer, inputSurface, outputSurface, flippedAngle,
                     percentMin, percentMax, *converter);
        outputTexture = updateTexture(renderer, outputSurface, outputTexture);
      }
      ImGui::EndDisabled();
    }

    ImGui::SameLine();

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

    // Zoom slider
    static float minDimension = 100;
    static float previewNum = std::min(8.0f, minDimension);
    // Slider for controlling the number of pixels in preview
    // Range from [1, min(width, height)] allowing full image previews
    ImGui::DragFloat("Pixels in section", &previewNum, 1.0f, 1.0f, minDimension,
                     "Pixels in section: %.0f", 0);
    previewNum = std::clamp(previewNum, 1.0f, minDimension);

    // Slider for controlling the size of the preview
    // Set the standard preview window size to 1/5 the min dimension of window
    static float previewSize =
        std::min(viewport->WorkSize.x, viewport->WorkSize.y) * 0.2;
    ImGui::DragFloat("Size of preview", &previewSize, 1.0f, 1.0f,
                     minDimension * 0.25, "Size of preview: %.0f", 0);

    displayTiledZoomableImages(viewport, renderer, inputSurface, inputTexture,
                               outputSurface, outputTexture, minDimension,
                               previewNum, previewSize);
  }
  ImGui::End();

  return 0;
}

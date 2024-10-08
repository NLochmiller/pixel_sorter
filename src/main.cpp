#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <string>

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

// Simple class, would be a struct, but constructors are nice
class QuantizerOptionItem {
public:
  QuantizerOptionItem(ColorConverter *function, std::string name,
                      std::string tooltip) {
    this->function = function;
    this->name = name;
    this->tooltip = tooltip;
  }
  ColorConverter *function; // ColorConverter function this repersents
  // Use char* instead of std::string as that is what DearImGui uses
  // This removes a step every frame
  std::string name;    // The name of this option (what is shown in the list)
  std::string tooltip; // The tooltip that is displayed over the item
};

// The options that repersent the pixel quantizers.
const QuantizerOptionItem quantizer_options[] = {
    /*
     * Organized by color spaces in this order:
     * - RGB
     * - HSV
     * - HSL
     * - Misc.
     */
    QuantizerOptionItem(&ColorConversion::red, "Red",
                        "The R in RGB of the pixel"),
    QuantizerOptionItem(&ColorConversion::green, "Green",
                        "The G in RGB of the pixel"),
    QuantizerOptionItem(&ColorConversion::blue, "Blue",
                        "The B in RGB of the pixel"),
    QuantizerOptionItem(&ColorConversion::hue, "Hue",
                        "The color shade of a pixel.\nThe H in HSV and HSL"),
    QuantizerOptionItem(
        &ColorConversion::saturation, "Saturation (HSV)",
        "How far from pure black a color appears to be.\nCalculated with the "
        "HSV color space, it is subtly different from the HSL Saturation.\nThe "
        "S in HSV"),
    QuantizerOptionItem(
        &ColorConversion::value, "Value",
        "The maximum of the RGB values of the pixel.\nThe V in HSV."),
    QuantizerOptionItem(
        &ColorConversion::saturation_HSL, "Saturation (HSL)",
        "How far from pure black a color appears to be.\nCalculated with the "
        "HSL color space, it is subtly different from the HSV Saturation.\n"
        "The S in HSL."),
    QuantizerOptionItem(&ColorConversion::lightness, "Lightness",
                        "How pale a color appears to be.\nThe L in HSL."),
    QuantizerOptionItem(&ColorConversion::average, "Average",
                        "The average of the RGB values of the pixel"),
    QuantizerOptionItem(&ColorConversion::minimum, "Minimum",
                        "The smallest of the RGB values of the pixel"),
    QuantizerOptionItem(&ColorConversion::maximum, "Maximum",
                        "The largest of the RGB values of the pixel"),
    QuantizerOptionItem(&ColorConversion::chroma, "Chroma",
                        "The difference between the maximum and minimum values "
                        "of the RGB values of the pixel.\n Effectivly: how "
                        "different a color is from the nearest gray")

};

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
void displayTiledZoomableImages(const ImGuiViewport *viewport,
                                SDL_Renderer *renderer,
                                SDL_Surface *&inputSurface,
                                SDL_Texture *&inputTexture,
                                SDL_Surface *&outputSurface,
                                SDL_Texture *&outputTexture, float minDimension,
                                float magnifier_pixels, float magnifier_size) {

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
      ImVec2 original_size = max_images_area; // To restore later

      /* Calculate size for the horizontal layout */
      max_images_area.x = (max_images_area.x - style.ItemSpacing.x) / 2;
      ImVec2 display_h =
          maximizeImVec2WithinBounds(input_image_scale, max_images_area);
      max_images_area = original_size;

      /* Calculate size for the vertical layout */
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
                             inputSurface->h, display.x, display.y,
                             magnifier_pixels, magnifier_size);
    }

    // Display vertical aspect images on the same line
    if (useHoriLayout) {
      ImGui::SameLine();
    }

    // Display output image zoomed in to percent
    if (outputTexture != NULL) {
      displayTextureZoomable(renderer, outputTexture, outputSurface->w,
                             outputSurface->h, display.x, display.y,
                             magnifier_pixels, magnifier_size);
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
      if (outputSurface != NULL) {
        IMG_SavePNG(outputSurface, outputPath.c_str());
      } else {
        fprintf(stderr, "The output image does not exist! You must sort before "
                        "exporting!\n");
      }
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
    ImGui::SeparatorText("Sorting settings");
    // Values used throughout this menu
    static float angle = 0;
    static float percentMin = 25.0;
    static float percentMax = 75.0;

    /* === Top options, sorting. ============================================ */
    static ImGuiTableFlags table_flags =
        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("Sort options menu", 2, table_flags)) {
      int column_id = 0;
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(column_id++);

      ImGui::Text("Sort by");
      ImGui::SameLine();
      static int selected_index = 7; // TODO: Use lightness as default
      /* Pixel quantizer selection */
      {
        static const int quantizers_count = arrayLen(quantizer_options);

        // Pass in the preview value visible before opening the combo
        const char *preview_value =
            quantizer_options[selected_index].name.c_str();

        static ImGuiComboFlags flags = 0;
        if (ImGui::BeginCombo("##PixelQuantizer", preview_value, flags)) {
          /* Display each item in combo */
          for (int n = 0; n < quantizers_count; n++) {
            const bool is_selected = (selected_index == n);
            if (ImGui::Selectable(quantizer_options[n].name.c_str(),
                                  is_selected))
              selected_index = n; // Update selected
            if (is_selected) // Set the initial focus when opening the combo
              ImGui::SetItemDefaultFocus();
            ImGui::SetItemTooltip("%s", quantizer_options[n].tooltip.c_str());
          }
          ImGui::EndCombo();
        }
        *converter = quantizer_options[selected_index].function; // update
      }
      ImGui::SetItemTooltip(
          "The value that each pixel in the image will be converted to and "
          "then sorted by.\nDefault is lightness");

      const ImGuiSliderFlags sliderFlags = ImGuiSliderFlags_AlwaysClamp;

      ImGui::Text("In the range ");
      ImGui::SameLine();
      // Set the minimum and maximum percentages of values will be sorted
      ImGui::DragFloatRange2("##Percentage range", &percentMin, &percentMax,
                             1.0f, 0.0f, 100.0f, "Minimum: %.2f%%",
                             "Maximum: %.2f%%", sliderFlags);
      ImGui::SetItemTooltip("The image will be sorted by %s that is\nin the "
                            "range %.2f to %.2f (inclusive).\nThese sliders "
                            "control the minimum and maximum of that range",
                            quantizer_options[selected_index].name.c_str(),
                            percentMin, percentMax);

      /* Sorting button. Enabled only when there is an input surface */
      ImGui::BeginDisabled(inputSurface == NULL);
      if (ImGui::Button("Sort")) {
        sort_wrapper(renderer, inputSurface, outputSurface, angle, percentMin,
                     percentMax, *converter);
        outputTexture = updateTexture(renderer, outputSurface, outputTexture);
      }
      ImGui::EndDisabled();

      /* === End of left half =============================================== */
      ImGui::TableSetColumnIndex(column_id++);
      /* === Right half. Get angle user wants to sort at ==================== */

      ImGui::Text("Angle");

      const static float low_rd = 0.0f;     // low value for radians & degrees
      const static float high_d = 360;      // High value for degrees
      const static float high_r = 2 * M_PI; // High value for radians
      std::string tooltip = "This controls the angle of the line that the "
                            "pixels of the image are sorted along.";

      // Scale the knob cicumference to 1/knob_scale of the
      // smallest dimension of window
      static float knob_scale = 10.0f;

      float knob_radius = std::min(viewport->WorkSize.x, viewport->WorkSize.y) /
                          (2 * knob_scale);
      float knob_angle = DEG_TO_RAD(angle);
      if (ImGui::Knob("##Sort angle knob", &knob_angle, knob_radius)) {
        // Knob has caused a change, update the angle
        angle = RAD_TO_DEG(knob_angle);
        std::clamp(angle, low_rd, high_r);
      }
      ImGui::SetItemTooltip("%s", tooltip.c_str());

      ImGui::SetNextItemWidth(knob_radius * 2);
      // Convert to human readable angle
      float displayAngle = std::clamp((float)(360 - angle), low_rd, high_d);
      if (ImGui::DragFloat("##Sort angle", &displayAngle, 1.0f, 0.0f, 360.0f,
                           "%.2f", sliderFlags | ImGuiSliderFlags_WrapAround)) {
        // There has been a change. Change the angle to repersent this
        // change in the display angle
        angle = (360 - displayAngle);
        std::clamp(angle, low_rd, high_d);
      }
      ImGui::SetItemTooltip("%s\nControl Left click to enter an angle.",
                            tooltip.c_str());
    }
    /* === End of angle options ============================================= */
    ImGui::EndTable();
    /* === End of top options =============================================== */

    /* Magnifier Setting Header */
    ImGui::SeparatorText("Magnifier Settings");
    ImGui::SetItemTooltip(
        "The magnifier will show a zoomed in section of the "
        "image the cursor is hovering over. Centered at the cursor.");

    static int minDimension = 100;
    static int magnifier_pixels = 8;
    // How much of the min dimension the preview size can take up
    const static float magnifier_preview_max_scale = 0.2;
    static int magnifier_preview_size =
        round((std::min(viewport->WorkSize.x, viewport->WorkSize.y) *
               magnifier_preview_max_scale));
    /*
     * Ideally the layout will be in 1 line, like this:
     * Number of pixels: [=====|===]     Magnified size: [====|===]
     */
    ImGui::BeginDisabled(inputSurface == NULL);
    {

      if (ImGui::BeginTable("##MagnifierSettingsTable", 2)) {
        /* Number of pixels in the magnifier preview */
        ImGui::TableNextColumn();
        if (inputSurface != NULL) {
          minDimension =
              std::min((int)(inputSurface->w), (int)(inputSurface->h));
        }
        // Range from [1, min(width, height)] allowing full image previews
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderInt("##MagnifierPixels", &magnifier_pixels, 1,
                         minDimension, "Pixels: %d",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::SetItemTooltip("How many pixels of the image are displayed in a "
                              "n by n square in the magnifier.\nControl Left "
                              "click to enter a value.");

        /* popup size TODO: Find a good name for this */
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderInt(
            "##MagnifierPreviewSize", &magnifier_preview_size, 1,
            round(std::min(viewport->WorkSize.x, viewport->WorkSize.y) *
                  magnifier_preview_max_scale),
            "Size: %d");

        ImGui::SetItemTooltip(
            "The size of the magnified section of the image on your "
            "screen.\nControl Left click to enter a value.");
        ImGui::EndTable();
      }
    }
    ImGui::EndDisabled();

    /* Image display */
    ImGui::SeparatorText("Source and sorted images");
    ImGui::SetItemTooltip(
        // Arbitrary line length of 50 chars, bar is at 50 |
        "The source and sorted image are displayed in either\n"
        "a horizontal or vertical format. The format chosen\n"
        "will maximize the space both images can take up\n"
        "in the window.\n\n"
        "If the layout is horizontal:\n"
        "    The original image is on the left\n"
        "    The sorted image is on the right\n\n"
        "If the layout is vertical:\n"
        "    The original image is on the top\n"
        "    The sorted image is on the bottom\n");
    displayTiledZoomableImages(viewport, renderer, inputSurface, inputTexture,
                               outputSurface, outputTexture,
                               (float)minDimension, (float)magnifier_pixels,
                               (float)magnifier_preview_size);
  }
  ImGui::End();

  return 0;
}

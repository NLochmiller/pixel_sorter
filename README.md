# Pixel Sorter
A graphics tool that applies a pixel sorting effect and allows for users to load/save images. Sorting pixels causes an interesting 'glitchy' blur like effect.
> If you cannot see the effect in this image, see the included [before and after](#comparison-of-before-and-after-sorting).

![An example image, a mountain lit by a sunrise or sunset, having been sorted by the pixel sorter](docs/mountain_sorted.png)


## How do you sort an image?
Since sorting is generally done in a 1d format, and not 2d images, we must convert the image into 1d arrays.
The image is first cut into lines such that each pixel is guaranteed to be along a single line (This is the line the angle controls refer to).
This requires that all lines are parallel, and that the lines are spaced just far apart from each other to have each pixel only be along 1 line.

To achieve this, all lines must be parallel, have no overlap, and when all lines are combined, cover every pixel in the image.
This is achieved by making each line a copy of a line generated using [Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm) at different offsets (spaced one pixel apart from each other) from a fixed spot on the image.
If for each line we sort the pixels along that line, then we have sorted the image.

### How to sort the pixels along a line
> [!NOTE]
> “value range” refers to the user chosen range of values that should be sorted, which is [ Range Minimum, Range Maximum ]. See [Controls](#controls) for information on Range Minimum and Maximum. 

Each pixel is converted to the value that the user chose *(for example, red)*. 
Then the line will be scanned, ignoring any pixels that are outside the value range, until a section of the line is found that is a contiguous set of values that are inside the value range, which is referred to as a span.

The span is essentially a 1d array, and will then be sorted by value, utilizing counting sort.
The process is repeated for each span in the line, until all spans are sorted, and thus all pixels along the line are sorted.


<!-- The effect is not always immediatly visible, so having a comparison like this is very helpful -->
## Comparison of before and after sorting
Notice how the sides of the mountain in the sorted image are blured, that is due to the pixels being sorted. 
> If you want more to look at more before and afters, see [the documents folder](docs) in this repository.
### Unsorted
![View of mountain, unsorted](docs/mountain_small_unsorted.png)
### Sorted
![View of mountain, sorted](docs/mountain_small_sorted.png)



# Usage
- Install/Build the program
- Go to File > Open
- Use the file manager to find a .png or .jpg file you want to sort
- Modify sort settings
- Press the "Sort" Button
- Once you are happy with the results go to File > Export as and choose what you want the sorted image to be saved as (currently only exports to the png format)



# Building
<!-- This top level of building looks chunky & clunky. TODO: Revise it -->
> [!Caution]
> This project was designed and tested only on/for Linux and Windows.

Make sure that you have [cmake](https://cmake.org/download/) installed.
For convenience, a empty [build directory](./build) is provided.

> If you want to change the version of SDL2, version 2.0.17 or higher of SDL2 is ***required***

## Linux
### 1. Install required packages
Using a package manager, install [SDL2](https://wiki.libsdl.org/SDL2/Installation) along with [SDL2_image](https://wiki.libsdl.org/SDL2_image/FrontPage). 
*If you install a developer package of SDL2, SDL2_image ****may**** be included.*

<details>
<summary>Debian based (includes Ubuntu)</summary>

  This installs SDL2 and SDL2_image
  ```
  sudo apt-get install libsdl2-dev
  ```
    
</details>
<details>
<summary>NixOS/nix</summary>
  
  A flake is provided for you in the base folder of this repository.
  To use the flake simply enter the following while in the base folder.
  ```
  nix-shell
  ```
  
</details>

### 2. Run the build command
From the build directory:
```
cmake ..
cmake --build . -j
```


## Windows
A local copy of [SDL2](./external/sdl/SDL2) *version 2.30.10* and [SDL2_image](./external/sdl/SDL2_image) *version 2.8.3* are included in this git repository.

### (Optional) Change the version of SDL2 and/or SDL2_image
<details>
<summary>SDL2</summary>
  
  Version 2.0.17 or higher of SDL2 is ***required***
  1. Locate the release for the version of SDL2 you want [here](https://github.com/libsdl-org/SDL/releases).
  2. Download SDL2-devel-VERSION-COMPILER.zip from that release.
   - For version 2.30.10 using Microsoft Visual Studio, you would download SDL2-devel-2.30.10-VC.zip
  3. Extract the zip folder, and copy its contents into the local [SDL2 folder](./external/sdl/SDL2)

</details>
<details>
<summary>SDL2_image</summary>
  
  1. Locate the release for the version of SDL2_image you want [here](https://github.com/libsdl-org/SDL_image/releases).
  2. Download SDL2_image-devel-VERSION-COMPILER.zip from that release.
   - For version 2.8.3 using Microsoft Visual Studio, you would download SDL2-devel-2.8.3-VC.zip
  3. Extract the zip folder, and copy its contents into the local [SDL2_image folder](./external/sdl/SDL2_image)

</details>

### 1. Run the build command
> [!Tip]
> To open command prompt in the curent folder, click on the path in file explorer (it should be just below the menu). Replace the path with `cmd` and hit enter.
>
> You can do this for powershell as well by entering `powershell` instead of `cmd`

In the build folder:
```
cmake ..
cmake --build .
```

### Fix for "The code execution cannot proceed because *.dll was not found" error
The SDL2.dll and SDL2_image.dll must be in the folder that the executable is located in.
To fix this change the `EXE_OUTPUT_FOLDER` to whatever folder the exe is being output to.

> [!Tip]
> ${CMAKE_BINARY_DIR} is a cmake variable that is equivalent to the folder you are building in.
> I recommend setting `EXE_OUTPUT_FOLDER` to be relative to this folder.

1. Go to [.\CMakeLists.txt](./CMakeLists.txt) and finding the following on line 9
```
set(EXE_OUTPUT_FOLDER "${CMAKE_BINARY_DIR}\\Debug")
```

2. Change the final part from `Debug` to the relative path to the folder your exe is being built into.
3. Run the build commands again

For example, if the executable ends up in `.\build\Release` we should change the line to be
```
set(EXE_OUTPUT_FOLDER "${CMAKE_BINARY_DIR}\\Release")
```



# Controls
> [!TIP]
> All controls have tool tips when the cursor hovers over them.

### Sorting terminology
- Value: What value of each pixel should be sorted, including Hue, Saturation, and Value.
- Range Minimum: Choose the minimum value that will be sorted
- Range Maximum: Choose the maximum value that will be sorted
- Angle knob and slider: Change the angle of the line the pixels are sorted along.

### Magnifier
When the mouse cursor is over the original or sorted image, a small magnified view of the image will show up, with the view centered on the cursor.
- Pixels: The width and height of the zoomed in area are controlled by this.
- Size: This controls the size of the popup on the screen.




# Dependencies included.
> There is no need to download these. The source code needed is contained in [the libraries folder](libs).
- [DearImGui](https://github.com/ocornut/imgui)
- [imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser)



# License
This project is licensed under the BSD 3-Clause License

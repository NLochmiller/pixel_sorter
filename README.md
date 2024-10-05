# Pixel Sorter
A graphics tool that modifies images by sorting pixels, leading to an interesting 'glitchy' blur like effect.

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



## Usage
- Install the program
- Go to File > Open
- Use the file manager to find a .png or .jpg file you want to sort
- Modify sort settings
- Press the "Sort" Button
- Once you are happy with the results go to File > Export as and choose what you want the sorted image to be saved as (currently only exports to the png format)


## Controls
> [!TIP]
> All controls have tool tips when the cursor hovers over them.
### Sorting
- Value: What value of each pixel should be sorted, including Hue, Saturation, and Value.
- Range Minimum: Choose the minimum value that will be sorted
- Range Maximum: Choose the maximum value that will be sorted
- Angle knob and slider: Change the angle of the line the pixels are sorted along.

### Magnifier
When the mouse cursor is over the original or sorted image, a small magnified view of the image will show up, with the view centered on the cursor.
- Pixels: The width and height of the zoomed in area are controlled by this.
- Size: This controls the size of the popup on the screen.

## Build Dependencies
SDL2 and SDL2 image, while this uses DearImGui, those files are included in this repository

> [!Caution]
> For now Windows is not supported. *I plan to add support in the future*


## License
This project is licensed under the BSD 3-Clause License

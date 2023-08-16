#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL.h>
#include <SDL_image.h>

#include "colorConversion.h"

void printArray(Uint8* arr, int len){
    printf("[");
    for(int i = 0; i < len-1; i++){
        printf("%3.1d, ", arr[i]);
    }
    printf("%3.1d]", arr[len-1]);
}

void printCount(unsigned long* count, int len){
    printf("Count: [");
    for(int i = 0; i < len-1; i++){
        printf("%ld, ", count[i]);
    }
    printf("%ld]\n", count[len-1]);

}

// This is here for reference
// Counting sort
// Sorts the first n ints of arr and puts them in out
// Assumes that arr and out are the same size
// To make it work with 0, must add 1 anytime arr[i] is called, except when
// writing its value to output
void countingSort(Uint8* arr, Uint8* out, int length, int n){
    // Count will store the count of each number
    unsigned long* count = (unsigned long*) calloc(UINT8_MAX+2,
                                                   sizeof(unsigned long));
    int i = 0;
    // Store count of each value
    for (i = 0; i < length; ++i){
        ++count[arr[i]+1];
    }

    // Change count[i] so that count[i] now contains actual
    // position of this value in output surface
    for (i = 1; i <= UINT8_MAX; ++i){
        count[i] += count[i-1];
    }

    // Build the output array
    for (i = 0; i < length; ++i){
        out[count[arr[i]+1]-1] = arr[i];
        --count[arr[i]+1];
    }

    free(count);

    // If we were putting the ints back in out, we would do that
}


// Maps [0,255] to [0,1]
double convert255to1(Uint8 n){
    return (0.0+n)/255.0;
}

// Convert rgb from [0,255] to [0,1]
void convertRGBfrom255to1(Uint8 r, Uint8 g, Uint8 b,
                          double* R, double* G, double* B){
    *R = convert255to1(r);
    *G = convert255to1(g);
    *B = convert255to1(b);
}


// Calculate the value of each pixel using valueFunc, and store the result
// at the corresponding index in values
void surfaceCalculateValues(double* values, unsigned long length,
                            SDL_Surface* imageIn,
                            double (*calcFunc)(double, double, double)){
    Uint32* pixels = imageIn->pixels;

    for (unsigned long i = 0; i < length; i++){
        Uint8 r, g, b, a;
        double R, G, B;

        SDL_GetRGBA(pixels[i], imageIn->format, &r, &g, &b, &a);

        // Convert from Uint8 to doubles
        convertRGBfrom255to1(r, g, b, &R, &G, &B);

        values[i] = calcFunc(R, G, B);
    }
}

// Calculate a contrast mask based on values, put output in output
void calculateContrastMask(double* doubleValues, bool* output,
                           unsigned long numPixels,
                           double conMin, double conMax){
    // For each pixel if conMin <= values <= conMax, set true else false
    for (unsigned long i = 0; i < numPixels; i++){
        if (doubleValues[i] >= conMin && doubleValues[i] <= conMax){
            output[i] = true;
        } else {
            output[i] = false;
        }
    }
}

// Convert each double to an int by multiplying them by some constant value
void convertFromDoublesToInts(double* doubleValues, Uint8* values,
                              unsigned long numPixels){
    for (unsigned long i = 0; i < numPixels; i++){
        values[i] = UINT8_MAX * doubleValues[i];
    }
}

void sortNPixels(Uint32* inPixels, Uint32* outPixels, unsigned long n,
                 Uint8* values){

    unsigned long countLen = UINT8_MAX+1;
    // Count will store the count of each number
    unsigned long* count = (unsigned long*) calloc(countLen,
                                                   sizeof(unsigned long));
    unsigned long i;

    // Store count of each value
    for (i = 0; i < n; ++i){
        ++count[values[i]];
    }

    // Change count[i] so that count[i] now contains actual
    // position of this value in output surface
    for (i = 1; i <= countLen; ++i){
        count[i] += count[i-1];
    }

    // Put the pixels where they should go
    // We want this to be stable to keep relative order of pixels the same
    // so if pixel 1 and 2 have the same value, they get put in as 1,2 not 2,1

    // unstable version for (unsigned long i = 0; i < n; ++i){

    // This would normally have i >= 0, but this will underflow to ULONG_MAX
    // and thus be an infinite loop, instead just have one be outside the loop
    for (unsigned long i = n-1; i > 0; i--){
        outPixels[count[values[i]] - 1] = inPixels[i];
        count[values[i]]--;
    }

    // Do the final placement, where i = 0
    outPixels[count[values[0]] - 1] = inPixels[0];
    count[values[0]]--;


    free(count);
}

// Sort the pixels based on the masked values
SDL_Surface* sortPixels(SDL_Surface* in, SDL_Surface* out, Uint8* values,
                        bool* valuesMask, unsigned long numPixels){
    // Having the same format allows for copying without having to change
    // their format and thus speed :)

    Uint32* inPixels = in->pixels;
    Uint32* outPixels = out->pixels;

    // Now do counting sort (Modified to output to a surface)
    for(int y = 0; y < in->h; y++){
        unsigned long lineStart = y * in->w;
        unsigned long spanStart = 0;
        bool lastMask = false;
        for(unsigned long x = 0; x < in->w; x++){
            unsigned long offset = lineStart + x;
            // At the start of the span, mark start
            if ( valuesMask[offset] == true && lastMask == false ){
                spanStart = x;
            }

            // Not part of a span, copy this pixel directly
            if ( valuesMask[offset] == false ){
                outPixels[offset] = inPixels[offset];
            }

            // At the end of the span or the end of the line which is in a span
            if ( (valuesMask[offset] == false && lastMask == true)
                 || ( x == in->w-1 && valuesMask[offset] == true ) ){
                unsigned long n = x - spanStart + 1;
                unsigned long spanOffset = spanStart + lineStart;

                sortNPixels(inPixels + spanOffset, outPixels + spanOffset,
                            n, values + spanOffset);
            }

            lastMask = valuesMask[offset];
        }
    }

    // If we were putting the ints back in out, we would do that
    return out;
}


int main(int argc, char** argv){
    const char* inputPath = "assets/eye.jpg";
    char* outputPath = "assets/outputs/output.png";
    double (*calcFunc)(double, double, double) = &calcHue;
    double conMin = 0;
    double conMax = 0.8;


    // Attempt to initialize graphics system
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Attempt to initialize the image system
    if (IMG_Init(IMG_INIT_PNG) == 0){
        fprintf(stderr, "Error initializing SDL_image: %s\n", SDL_GetError());
        return -2;
    }

    // Open the image
    SDL_Surface* imageIn = IMG_Load(inputPath);

    if (imageIn == NULL){
        fprintf(stderr, "Error loading input image.\n");
        IMG_Quit();
        SDL_Quit();
        return -2;
    }

    // Create the output image
    SDL_Surface* imageOut =
        SDL_CreateRGBSurfaceWithFormat(0, imageIn->w, imageIn->h, 8,
                                       SDL_PIXELFORMAT_RGBA32);
    if (imageIn == NULL){
        fprintf(stderr, "Error creating output surface: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -2;
    }

    // Convert imageIn to have the same formatting as imageOut
    // Doing this ensures that in and out have the same formatting, making
    // doing things faster
    SDL_Surface* oldSurface = imageIn;
    imageIn = SDL_ConvertSurface(imageIn, imageOut->format, 0);
    SDL_FreeSurface(oldSurface);
    if (imageIn == NULL){
        fprintf(stderr, "Error converting input surface: %s\n", SDL_GetError());
        return -2;
    }

    // Create the value array
    unsigned long numPixels = imageIn->w * imageIn->h;
    double* doubleValues = (double*) calloc(numPixels, sizeof(double));
    Uint8* values = (Uint8*) calloc(numPixels, sizeof(Uint8));

    // Fill out the values
    surfaceCalculateValues(doubleValues, numPixels, imageIn, calcFunc);
    convertFromDoublesToInts(doubleValues, values, numPixels);

    // Get the contrast mask
    bool* valuesMask =(bool*) calloc(numPixels, sizeof(bool));
    calculateContrastMask(doubleValues, valuesMask, numPixels, conMin, conMax);


    // Sort!
    sortPixels(imageIn, imageOut, values, valuesMask,
                                       numPixels);

    // Save the image
    int result = IMG_SavePNG(imageOut, outputPath);
    if ( result != 0 ){
        fprintf(stderr, "Error saving image.\n");
        IMG_Quit();
        SDL_Quit();
        return -3;
    }



    IMG_Quit();
    SDL_Quit();
    fprintf(stderr, "Exiting normally.\n");
    return 0;
}


/*
  The plan:

  1.) Using a function convert rgb to one value, ie, use some funciton f
  on every pixel so that Pixel[length] -> Values[length] by f(r,g,b)

  2.a) Put this through a contrast mask, or:
    -------------------------------
    | for each Value V in Values: |
    |   if V > min & V < max:     |
    |     ContrastMask[i] = 255   |
    |   else                      |
    |     ContrastMask[i] = 0     |
    |     see 2.b                 |
    -------------------------------

  2.b) Sort, every time we would usually write to the output, instead write to a
       surface which will have the pixels copied to it :)

  3.) Expand on this to support different sorting directions,


 */

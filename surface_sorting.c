#include <stdbool.h>
#include <SDL2/SDL.h>

#include "constants.h"


void output_sortRight(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                      Uint8* values, COUNT_T* count){
    // This would normally have i >= 0, but this will underflow to ULONG_MAX
    // and thus be an infinite loop, instead just have i=0 be outside the loop
    for (COUNT_T i = n-1; i > 0; i--){

        outPixels[count[values[i]] - 1] = inPixels[i];
        count[values[i]]--;
    }


    // Do the final placement, where i = 0
    outPixels[count[values[0]] - 1] = inPixels[0];
    count[values[0]]--;
}


void output_sortLeft(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                     Uint8* values, COUNT_T* count){
    // This would normally have i >= 0, but this will underflow to ULONG_MAX
    // and thus be an infinite loop, instead just have i=0 be outside the loop
    for (COUNT_T i = 0; i < n; ++i){
        //for (COUNT_T i = n-1; i > 0; i--){
        outPixels[n - (count[values[i]])] = inPixels[i];
        count[values[i]]--;
    }

}


void sortPixelsRow(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                   Uint8* values, enum direction direction){

    COUNT_T countLen = UINT8_MAX+1;
    // Count will store the count of each number
    COUNT_T* count = (COUNT_T*) calloc(countLen,
                                                   sizeof(COUNT_T));
    COUNT_T i;

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
    if ( direction == left ){
        output_sortLeft(inPixels, outPixels, n, values, count);
    } else {
        output_sortRight(inPixels, outPixels, n, values, count);
    }

    free(count);
}

// Sort the pixels horizontally
void sortPixelsHorizontally(Uint32* inPixels, Uint32* outPixels,
                            Uint8* values, bool* valuesMask,
                            COUNT_T w, COUNT_T h, enum direction direction){

    // For each row
    for(int y = 0; y < h; y++){
        COUNT_T lineStart = y * w;
        COUNT_T spanStart = 0;
        bool lastMask = false;

        // For each pixel in this row
        for(COUNT_T x = 0; x < w; x++){
            COUNT_T offset = lineStart + x;

            // At the start of the span, mark the start
            if ( valuesMask[offset] == true && lastMask == false ){
                spanStart = x;
            }

            // This is not part of a span, copy this pixel
            if ( valuesMask[offset] == false ){
                outPixels[offset] = inPixels[offset];
            }

            // At the end of the span or the end of the line which is in a span
            if ( (valuesMask[offset] == false && lastMask == true)
                 || ( x == w-1 && valuesMask[offset] == true ) ){
                COUNT_T n = x - spanStart + 1;
                COUNT_T spanOffset = spanStart + lineStart;

                sortPixelsRow(inPixels + spanOffset, outPixels + spanOffset,
                              n, values + spanOffset, direction);
            }

            // Store this current mask value
            lastMask = valuesMask[offset];
        }
    }
}

void output_sortDown(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                     Uint8* values, COUNT_T* count, COUNT_T w){
    // This would normally have i >= 0, but this will underflow to ULONG_MAX
    // and thus be an infinite loop, instead just have i=0 be outside the loop
    COUNT_T i = n-1;
    for (; i > 0; i--){

        outPixels[w*(count[values[i*w]] - 1)] = inPixels[i*w];
        count[values[i * w]]--;

    }


    // Do the final placement, where i = 0
    outPixels[w*(count[values[0]] - 1)] = inPixels[0];
    count[values[0]]--;
}


void output_sortUp(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                   Uint8* values, COUNT_T* count, COUNT_T w){
    // This would normally have i >= 0, but this will underflow to ULONG_MAX
    // and thus be an infinite loop, instead just have i=0 be outside the loop
    for (COUNT_T i = 0; i < n; ++i){
        //for (COUNT_T i = n-1; i > 0; i--){
        outPixels[w*(n - (count[values[i * w]]))] = inPixels[i*w];
        count[values[i* w]]--;
    }
}


void sortPixelsColumn(Uint32* inPixels, Uint32* outPixels, COUNT_T n,
                      Uint8* values, COUNT_T w, enum direction direction){

    COUNT_T countLen = UINT8_MAX+1;
    // Count will store the count of each number
    COUNT_T* count = (COUNT_T*) calloc(countLen, sizeof(COUNT_T));
    COUNT_T i;

    // Store count of each value
    for (i = 0; i < n; ++i){
        ++count[values[i * w]];
    } 


    // Change count[i] so that count[i] now contains actual
    // position of this value in output surface
    for (i = 1; i <= countLen; ++i){
        count[i] += count[i-1];
    }

    // Put the pixels where they should go
    // We want this to be stable to keep relative order of pixels the same
    // so if pixel 1 and 2 have the same value, they get put in as 1,2 not 2,1
    if ( direction == down ){
        output_sortDown(inPixels, outPixels, n, values, count, w);
    } else {
        output_sortUp(inPixels, outPixels, n, values, count, w);
    }

    free(count);
}

// Sort the pixels vertically
void sortPixelsVertically(Uint32* inPixels, Uint32* outPixels,
                            Uint8* values, bool* valuesMask,
                            COUNT_T w, COUNT_T h, enum direction direction){

    // For each column
    for(int x = 0; x < w; x++){
        COUNT_T lineStart = x;
        COUNT_T spanStart = 0;
        bool lastMask = false;

        // For each pixel in this row
        for(COUNT_T y = 0; y < h; y++){


            COUNT_T offset = lineStart + y * w;

            // At the start of the span, mark the start
            if ( valuesMask[offset] == true && lastMask == false ){
                spanStart = y;
            }

            // This is not part of a span, copy this pixel
            if ( valuesMask[offset] == false ){
                outPixels[offset] = inPixels[offset];
            }

            // At the end of the span or the end of the line which is in a span
            if ( (valuesMask[offset] == false && lastMask == true)
                 || ( y == h-1 && valuesMask[offset] == true ) ){
                COUNT_T n = y - spanStart + 1;
                COUNT_T spanOffset = spanStart * w + lineStart;
                
                sortPixelsColumn(inPixels + spanOffset, outPixels + spanOffset,
                                 n, values + spanOffset, w, direction);
            }

            // Store this current mask value
            lastMask = valuesMask[offset];
        }
    }
}


// Sort the pixels based on the masked values
SDL_Surface* sortPixels(SDL_Surface* in, SDL_Surface* out, Uint8* values,
                        bool* valuesMask, COUNT_T numPixels,
                        enum direction direction){


    // Having the same format allows for copying without having to change
    // their format and thus speed :)

    Uint32* inPixels = in->pixels;
    Uint32* outPixels = out->pixels;
    if ( direction == left || direction == right ){
        sortPixelsHorizontally(inPixels, outPixels, values, valuesMask,
                               in->w, in->h, direction);
    } else {
        sortPixelsVertically(inPixels, outPixels, values, valuesMask,
                               in->w, in->h, direction);
    }

    // If we were putting the ints back in out, we would do that
    return out;
}

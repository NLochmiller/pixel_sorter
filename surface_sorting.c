#include <stdbool.h>
#include <SDL2/SDL.h>

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

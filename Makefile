# The input and outputs filenames
IN_FILE = pixel_sorter.c
OUT_FILE = pixel_sorter

# The compiler
CC = gcc
# Compiler flags
CFLAGS := -g -Wall
# Needed for linking libraries, such as -lm for math.h for c
LINKS :=  `sdl2-config --libs --cflags` -lSDL2_image -lm

# The .c files we want to use (HELPER FUNCTIONS)
CFILES := color_conversion.c surface_sorting.c 
HFILES := constants.h color_conversion.h surface_sorting.h 

# cc flags output input *.h *.c links
$(OUT_FILE): $(IN_FILE) $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) $(IN_FILE) -o $(OUT_FILE) $(HFILES) $(CFILES) $(LINKS)

run: $(OUT_FILE)
	./$(OUT_FILE)
	open ./output.png

clean:
	rm -f $(OUT_FILE) *.o *.out output.png

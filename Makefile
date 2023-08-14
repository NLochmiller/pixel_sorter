# The input and outputs filenames
IN_FILE = pixel_sorter.c
OUT_FILE = pixel_sorter

# The compiler
CC = gcc
# Compiler flags
CFLAGS := -g -Wall
# Needed for linking libraries, such as -lm for math.h for c
LINKS :=  `sdl2-config --libs --cflags` -lm

# The .c files we want to use (HELPER FUNCTIONS)
CFILES :=
HFILES :=

# cc flags output input *.h *.c links
$(OUT_FILE): $(IN_FILE) $(CFILES) $(HFILES)
	$(CC) $(CFLAGS) $(IN_FILE) -o $(OUT_FILE) $(HFILES) $(CFILES) $(LINKS)

run: $(OUT_FILE)
	./$(OUT_FILE)

clean:
	rm -f $(OUT_FILE) *.o *.out

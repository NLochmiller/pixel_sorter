CXX_VERSION=17
OUTPUT = pixel_sorter

SRC_DIR = ./src
LIBS_DIR = ./libs
IMGUI_DIR = $(LIBS_DIR)/imgui


# Normal sources
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

# Add the imgui files
SOURCES += $(wildcard $(IMGUI_DIR)/*.cpp)

# Add the imgui backend files for SDL2
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXX = g++
CXXFLAGS = -std=c++$(CXX_VERSION) -I $(IMGUI_DIR) -I $(IMGUI_DIR)/backends     \
	-I $(LIBS_DIR)/file_browser -I $(SRC_DIR)
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += `sdl2-config --cflags --libs`

LIBS = -lGL -ldl -lSDL2_image `sdl2-config --libs`

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

all: $(OUTPUT)
	@echo Build complete

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

run: $(OUTPUT)
	./$(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJS) *.o

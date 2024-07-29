CXX_VERSION=17
OUTPUT = pixel_sorter
# Normal sources
SOURCES = LineInterpolator.cpp main.cpp

LIBS_DIR = ./libs
IMGUI_DIR = $(LIBS_DIR)/imgui

# Add the imgui main files
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp\
	$(IMGUI_DIR)/imgui_widgets.cpp
# Add the imgui backend files for SDL2
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXX = g++
CXXFLAGS = -std=c++$(CXX_VERSION) -I $(IMGUI_DIR) -I $(IMGUI_DIR)/backends     \
	-I $(LIBS_DIR)/file_browser -I ./
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += `sdl2-config --cflags --libs`

LIBS = -lGL -ldl -lSDL2_image `sdl2-config --libs`

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OUTPUT)
	@echo Build complete

$(OUTPUT): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

run: $(OUTPUT)
	./$(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJS)

# CekyMonitor Makefile
CXX = g++
CXX_WIN = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -I./lib/imgui -I./lib
INCLUDES_WIN = -I./lib/imgui -I./lib/glfw-win
LIBS = -lglfw -lGL -lX11 -lpthread -ldl
LIBS_WIN = -L./lib/glfw-win -lglfw3 -lopengl32 -lgdi32 -liphlpapi -lpsapi -lpdh -static

# ImGui source files
IMGUI_SRC = lib/imgui/imgui.cpp \
            lib/imgui/imgui_draw.cpp \
            lib/imgui/imgui_tables.cpp \
            lib/imgui/imgui_widgets.cpp \
            lib/imgui/imgui_impl_glfw.cpp \
            lib/imgui/imgui_impl_opengl3.cpp

# Main application
MONITOR_SRC = src/main.cpp
MONITOR_WIN_SRC = src/main_win.cpp
SERVER_SRC = src/server.cpp

# Output
BUILD_DIR = build
MONITOR_OUT = $(BUILD_DIR)/monitor
MONITOR_WIN_OUT = $(BUILD_DIR)/CekyMonitor.exe
SERVER_OUT = $(BUILD_DIR)/server

.PHONY: all clean monitor server windows

all: monitor server

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

monitor: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MONITOR_SRC) $(IMGUI_SRC) -o $(MONITOR_OUT) $(LIBS)
	@echo "✓ CekyMonitor (Linux) built successfully!"

windows: $(BUILD_DIR)
	$(CXX_WIN) $(CXXFLAGS) $(INCLUDES_WIN) $(MONITOR_WIN_SRC) $(IMGUI_SRC) -o $(MONITOR_WIN_OUT) $(LIBS_WIN)
	@echo "✓ CekyMonitor.exe (Windows) built successfully!"

server: $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o $(SERVER_OUT)
	@echo "✓ Server built successfully!"

clean:
	rm -rf $(BUILD_DIR)
	@echo "✓ Cleaned!"

run-monitor: monitor
	./$(MONITOR_OUT)

run-server: server
	./$(SERVER_OUT)

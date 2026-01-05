# Eray Monitor v5.0

A modern system monitoring application for Linux and Windows.

## Features

- CPU: Model, cores, frequency, temperature, usage
- RAM: Memory and swap usage
- GPU: NVIDIA/AMD/Intel support
- Disk: Partitions and I/O speeds
- Network: Download/upload speeds
- Battery: Status and power consumption
- Modern dark theme UI with real-time graphs

## Building

### 1. Install Dependencies

Ubuntu/Debian:
```bash
sudo apt install build-essential libglfw3-dev libgl1-mesa-dev
```

### 2. Setup ImGui Library

```bash
cd lib
git clone --depth 1 https://github.com/ocornut/imgui.git
cp imgui/backends/imgui_impl_glfw.* imgui/
cp imgui/backends/imgui_impl_opengl3* imgui/
```

### 3. Build

```bash
make monitor        # Linux
make windows        # Windows (requires mingw-w64)
```

### 4. Run

```bash
./build/monitor
```

## License

MIT

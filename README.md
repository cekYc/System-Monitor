# Ceky Monitor v6.0

A premium, real-time system monitoring application for **Linux** and **Windows** built with ImGui + GLFW + OpenGL3.

![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-blue)
![Version](https://img.shields.io/badge/Version-6.0-green)
![UI](https://img.shields.io/badge/UI-ImGui-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Features

### Overview Tab
| Metric | Linux | Windows |
|--------|:-----:|:-------:|
| CPU Usage (total + per-core) | ✅ | ✅ |
| CPU Temperature | ✅ | ✅ |
| CPU Frequency (current/max) | ✅ | ✅ |
| CPU Model & Core Count (logical/physical) | ✅ | ✅ |
| Peak CPU Tracking | ✅ | ✅ |
| RAM Usage (Used/Total/Percent) | ✅ | ✅ |
| RAM Peak Tracking | ✅ | ✅ |
| Memory Detail (Cached, Buffers, Dirty) | ✅ | ✅ |
| Swap Usage | ✅ | ✅ |
| GPU Usage, Temp, VRAM | ✅ | ✅ |
| GPU Fan Speed & Power Draw | ✅ | ✅ |
| Network Download/Upload Speed | ✅ | ✅ |
| Network Session Totals (RX/TX) | ✅ | ✅ |
| TCP Connection Count | ✅ | ✅ |
| Disk I/O (Read/Write Speed + Totals) | ✅ | ✅ |
| Storage Partitions (Usage bars) | ✅ | ✅ |
| Filesystem Type Display | ✅ | - |
| Battery (%, Charging, Power, Est. Time) | ✅ | ✅ |
| Load Averages (1/5/15 min) | ✅ | - |
| Context Switches/sec | ✅ | - |
| Interrupts/sec | ✅ | - |
| IO Wait % with history graph | ✅ | - |

### Processes Tab
| Feature | Linux | Windows |
|---------|:-----:|:-------:|
| Top 20-25 Processes | ✅ | ✅ |
| Per-process CPU % | ✅ | ✅ |
| Per-process Memory | ✅ | ✅ |
| Per-process Thread Count | ✅ | ✅ |
| Process State (R/S/D/Z) | ✅ | - |
| Total Process Count | ✅ | ✅ |
| Total Thread Count | ✅ | ✅ |
| File Descriptor Count | ✅ | ✅ |
| Handle Count | - | ✅ |

### System Info Tab
| Info | Linux | Windows |
|------|:-----:|:-------:|
| OS Name & Version | ✅ | ✅ |
| Distribution Name | ✅ | - |
| Kernel Version | ✅ | - |
| Architecture | ✅ | ✅ |
| Hostname | ✅ | ✅ |
| Username | ✅ | ✅ |
| Uptime | ✅ | ✅ |
| Detailed Memory Breakdown | ✅ | ✅ |
| All Storage Partitions | ✅ | ✅ |
| GPU Details | ✅ | ✅ |
| Network Session Stats | ✅ | ✅ |

### UI Features
- Modern dark glassmorphism theme with neon accent colors
- Animated header with pulsing title
- Card-based layout with border glow effects
- Gradient progress bars with overlay text
- Circular progress indicators
- Real-time line graphs (120-sample history)
- Per-core CPU mini bars
- Sortable process table
- 3-tab navigation (Overview / Processes / System Info)
- Status bar footer with live summary

## Building

### Linux

#### 1. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential libglfw3-dev libgl1-mesa-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ glfw-devel mesa-libGL-devel
```

**Arch:**
```bash
sudo pacman -S base-devel glfw-x11 mesa
```

#### 2. Setup ImGui Library

```bash
cd lib
git clone --depth 1 https://github.com/ocornut/imgui.git
cp imgui/backends/imgui_impl_glfw.* imgui/
cp imgui/backends/imgui_impl_opengl3* imgui/
```

#### 3. Build & Run

```bash
make monitor
./build/monitor
```

### Windows

#### Prerequisites
- [MSYS2](https://www.msys2.org/) with MinGW-w64 toolchain
- GLFW3 (`pacman -S mingw-w64-x86_64-glfw`)

#### Build with PowerShell
```powershell
# Build only
.\build.ps1

# Build and run
.\build.ps1 -Run
```

#### Build with Makefile (requires MSYS2 make)
```bash
make windows
```

## Project Structure

```
├── src/
│   ├── main.cpp          # Linux version
│   └── main_win.cpp      # Windows version
├── lib/
│   └── imgui/            # ImGui library files
├── build/
│   ├── monitor           # Linux binary
│   └── CekyMonitor.exe   # Windows binary
├── Makefile              # Cross-platform build
├── build.ps1             # Windows PowerShell build script
└── README.md
```

## Data Sources

### Linux
- `/proc/stat` — CPU usage, per-core, IO wait, context switches, interrupts
- `/proc/meminfo` — Detailed memory: Total, Available, Cached, Buffers, Dirty, Swap, Active/Inactive
- `/proc/net/dev` — Network interface traffic
- `/proc/net/tcp` + `/proc/net/tcp6` — TCP connection count
- `/proc/diskstats` — Disk I/O counters
- `/proc/mounts` + `statvfs()` — Partition usage
- `/proc/[pid]/stat` — Per-process CPU, memory, threads, state
- `/proc/loadavg` — Load averages
- `/proc/uptime` — System uptime
- `/proc/cpuinfo` — CPU model, core count
- `/proc/sys/fs/file-nr` — File descriptor count
- `/sys/class/thermal/` — CPU temperature
- `/sys/devices/system/cpu/` — CPU frequency
- `/sys/class/power_supply/` — Battery info
- `/sys/class/drm/` — AMD/Intel GPU fallback
- `/etc/os-release` — Distribution name
- `uname()` — Kernel version, architecture
- `nvidia-smi` — NVIDIA GPU metrics (temp, usage, VRAM, fan, power)

### Windows
- Windows API (PSAPI, IPHLPAPI, TlHelp32, WinIoCtl)
- Performance counters for CPU, memory, processes
- Registry for OS version detection (Windows 11 support)

## License

MIT


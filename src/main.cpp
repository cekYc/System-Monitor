// ErayMonitor v5.0 - Modern Linux Sistem Monitörü
// Özellikler: CPU, RAM, GPU, Disk, Network, Pil, Frekans, İşlemler, Uptime
// Modern UI: Glassmorphism, Neon renkler, Animasyonlar, Kartlar

#include <cfloat>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <dirent.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <cmath>
#include <GLFW/glfw3.h>

// ==================== YAPILAR ====================
struct CPUStats { long long user, nice, system, idle, iowait, irq, softirq, steal; };
struct NetStats { long long rxBytes, txBytes; };
struct DiskStats { long long readBytes, writeBytes; };
struct DiskInfo { std::string name; float totalGB, usedGB, usagePercent; };
struct GPUInfo { std::string name; int temp; int usagePercent; int memUsedMB, memTotalMB; bool available; };
struct BatteryInfo { int percent; bool charging; bool available; float powerWatts; };

// ==================== RENK PALETİ ====================
namespace Colors {
    // Ana renkler
    const ImVec4 Background = ImVec4(0.06f, 0.06f, 0.09f, 1.0f);
    const ImVec4 CardBg = ImVec4(0.11f, 0.11f, 0.14f, 0.95f);
    const ImVec4 CardBgHover = ImVec4(0.14f, 0.14f, 0.18f, 0.95f);
    
    // Neon Renkler
    const ImVec4 Cyan = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);
    const ImVec4 Purple = ImVec4(0.7f, 0.3f, 1.0f, 1.0f);
    const ImVec4 Pink = ImVec4(1.0f, 0.4f, 0.7f, 1.0f);
    const ImVec4 Green = ImVec4(0.2f, 1.0f, 0.5f, 1.0f);
    const ImVec4 Orange = ImVec4(1.0f, 0.6f, 0.2f, 1.0f);
    const ImVec4 Yellow = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
    const ImVec4 Red = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    const ImVec4 Blue = ImVec4(0.3f, 0.5f, 1.0f, 1.0f);
    
    // Metin
    const ImVec4 TextPrimary = ImVec4(0.95f, 0.95f, 0.98f, 1.0f);
    const ImVec4 TextSecondary = ImVec4(0.6f, 0.6f, 0.7f, 1.0f);
    const ImVec4 TextMuted = ImVec4(0.4f, 0.4f, 0.5f, 1.0f);
}

// ==================== STİL AYARLARI ====================
void SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Genel
    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.TabRounding = 6.0f;
    style.ChildRounding = 10.0f;
    style.PopupRounding = 8.0f;
    
    // Padding ve Spacing
    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(12, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 20.0f;
    
    // Border
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.ChildBorderSize = 0.0f;
    
    // Scrollbar
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    
    // Renkler
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = Colors::Background;
    colors[ImGuiCol_ChildBg] = Colors::CardBg;
    colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.13f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.25f, 0.5f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    colors[ImGuiCol_Text] = Colors::TextPrimary;
    colors[ImGuiCol_TextDisabled] = Colors::TextMuted;
    
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.24f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    
    colors[ImGuiCol_TitleBg] = Colors::CardBg;
    colors[ImGuiCol_TitleBgActive] = Colors::CardBg;
    colors[ImGuiCol_TitleBgCollapsed] = Colors::CardBg;
    
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.1f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.45f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
    
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);
    
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.25f, 0.6f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.3f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    
    colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.2f, 0.25f, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = Colors::Cyan;
    colors[ImGuiCol_SeparatorActive] = Colors::Cyan;
    
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    
    colors[ImGuiCol_PlotLines] = Colors::Cyan;
    colors[ImGuiCol_PlotLinesHovered] = Colors::Pink;
    colors[ImGuiCol_PlotHistogram] = Colors::Purple;
    colors[ImGuiCol_PlotHistogramHovered] = Colors::Pink;
}

// ==================== ÖZEL ÇİZİM FONKSİYONLARI ====================

// Gradient Progress Bar
void DrawGradientProgressBar(float fraction, ImVec2 size, ImVec4 colorLeft, ImVec4 colorRight, const char* overlay = nullptr) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 endPos = ImVec2(pos.x + size.x, pos.y + size.y);
    
    if (size.x < 0) {
        size.x = ImGui::GetContentRegionAvail().x;
        endPos.x = pos.x + size.x;
    }
    
    // Arka plan
    window->DrawList->AddRectFilled(pos, endPos, IM_COL32(30, 30, 35, 255), 6.0f);
    
    // Progress
    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        ImVec2 fillEnd = ImVec2(pos.x + size.x * fraction, endPos.y);
        ImU32 colL = ImGui::ColorConvertFloat4ToU32(colorLeft);
        ImU32 colR = ImGui::ColorConvertFloat4ToU32(colorRight);
        window->DrawList->AddRectFilledMultiColor(pos, fillEnd, colL, colR, colR, colL);
    }
    
    // Overlay text
    if (overlay) {
        ImVec2 textSize = ImGui::CalcTextSize(overlay);
        ImVec2 textPos = ImVec2(pos.x + (size.x - textSize.x) * 0.5f, pos.y + (size.y - textSize.y) * 0.5f);
        window->DrawList->AddText(textPos, IM_COL32(255, 255, 255, 255), overlay);
    }
    
    ImGui::ItemSize(size);
}

// Circular Progress
void DrawCircularProgress(float fraction, float radius, ImVec4 color, const char* label, const char* value) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    
    // Arka plan çemberi
    window->DrawList->AddCircle(center, radius, IM_COL32(40, 40, 50, 255), 32, 4.0f);
    
    // Progress arc
    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        float startAngle = -3.14159f / 2.0f;
        float endAngle = startAngle + fraction * 2.0f * 3.14159f;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
        
        int segments = (int)(fraction * 32);
        if (segments < 3) segments = 3;
        
        for (int i = 0; i < segments; i++) {
            float a1 = startAngle + (endAngle - startAngle) * (float)i / segments;
            float a2 = startAngle + (endAngle - startAngle) * (float)(i + 1) / segments;
            ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
            ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
            window->DrawList->AddLine(p1, p2, col, 6.0f);
        }
    }
    
    // Değer ve etiket
    ImVec2 valueSize = ImGui::CalcTextSize(value);
    window->DrawList->AddText(ImVec2(center.x - valueSize.x * 0.5f, center.y - 10), IM_COL32(255, 255, 255, 255), value);
    
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    window->DrawList->AddText(ImVec2(center.x - labelSize.x * 0.5f, center.y + 8), IM_COL32(150, 150, 160, 255), label);
    
    ImGui::ItemSize(ImVec2(radius * 2, radius * 2));
}

// Kart başlığı
void CardHeader(const char* icon, const char* title, ImVec4 color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%s", icon);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", title);
    ImGui::Spacing();
}

// Mini grafik
void MiniGraph(const char* id, float* data, int count, ImVec4 color, float height = 40.0f) {
    ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.12f, 0.8f));
    ImGui::PlotLines(id, data, count, 0, NULL, 0, 100, ImVec2(-1, height));
    ImGui::PopStyleColor(2);
}

// ==================== CPU FONKSİYONLARI ====================
CPUStats readCPUStats() {
    std::ifstream file("/proc/stat");
    std::string line;
    CPUStats stats = {0};
    if (file.is_open()) {
        getline(file, line);
        std::stringstream ss(line);
        std::string label;
        ss >> label >> stats.user >> stats.nice >> stats.system >> stats.idle 
           >> stats.iowait >> stats.irq >> stats.softirq >> stats.steal;
    }
    return stats;
}

float getCPUTemp() {
    const char* paths[] = {
        "/sys/class/thermal/thermal_zone0/temp",
        "/sys/class/hwmon/hwmon0/temp1_input",
        "/sys/class/hwmon/hwmon1/temp1_input",
        "/sys/class/hwmon/hwmon2/temp1_input"
    };
    for (const char* path : paths) {
        std::ifstream file(path);
        if (file.is_open()) {
            long long temp;
            file >> temp;
            if (temp > 1000) return temp / 1000.0f;
            return (float)temp;
        }
    }
    return 0.0f;
}

float getCPUFrequency() {
    std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (file.is_open()) {
        long long freq;
        file >> freq;
        return freq / 1000.0f;
    }
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                return std::stof(line.substr(pos + 1));
            }
        }
    }
    return 0.0f;
}

float getCPUMaxFrequency() {
    std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (file.is_open()) {
        long long freq;
        file >> freq;
        return freq / 1000.0f;
    }
    return 0.0f;
}

int getCPUCoreCount() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.find("processor") == 0) count++;
    }
    return count > 0 ? count : 1;
}

std::string getCPUModel() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    while (getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 2);
                if (model.length() > 35) model = model.substr(0, 32) + "...";
                return model;
            }
        }
    }
    return "Unknown CPU";
}

// ==================== RAM FONKSİYONLARI ====================
float getRamUsage(float& total, float& available, float& swapTotal, float& swapUsed) {
    std::ifstream file("/proc/meminfo");
    std::string line;
    long long memTotal = 0, memAvail = 0, swapT = 0, swapFree = 0;
    
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string label;
        long long value;
        ss >> label >> value;
        
        if (label == "MemTotal:") memTotal = value;
        else if (label == "MemAvailable:") memAvail = value;
        else if (label == "SwapTotal:") swapT = value;
        else if (label == "SwapFree:") swapFree = value;
    }
    
    total = memTotal / (1024.0 * 1024.0);
    available = memAvail / (1024.0 * 1024.0);
    swapTotal = swapT / (1024.0 * 1024.0);
    swapUsed = (swapT - swapFree) / (1024.0 * 1024.0);
    
    return (memTotal > 0) ? ((memTotal - memAvail) / (float)memTotal) : 0;
}

// ==================== NETWORK FONKSİYONLARI ====================
NetStats readNetStats() {
    std::ifstream file("/proc/net/dev");
    std::string line;
    NetStats total = {0, 0};
    
    getline(file, line); getline(file, line);
    while (getline(file, line)) {
        if (line.find("lo:") != std::string::npos) continue;
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) line[colon] = ' ';
        
        std::stringstream ss(line);
        std::string iface;
        long long rx = 0, tx = 0, junk = 0;
        ss >> iface >> rx >> junk >> junk >> junk >> junk >> junk >> junk >> junk >> tx;
        
        if (!ss.fail()) {
            total.rxBytes += rx;
            total.txBytes += tx;
        }
    }
    return total;
}

// ==================== DİSK FONKSİYONLARI ====================
DiskStats readDiskStats() {
    std::ifstream file("/proc/diskstats");
    std::string line;
    DiskStats total = {0, 0};
    
    while (getline(file, line)) {
        std::stringstream ss(line);
        int major, minor;
        std::string name;
        long long reads, readsMerged, sectorsRead, readTime;
        long long writes, writesMerged, sectorsWritten, writeTime;
        
        ss >> major >> minor >> name >> reads >> readsMerged >> sectorsRead >> readTime
           >> writes >> writesMerged >> sectorsWritten >> writeTime;
        
        if (name.find("loop") != std::string::npos) continue;
        if (name.length() == 3 || (name.find("nvme") != std::string::npos && name.find("p") == std::string::npos)) {
            total.readBytes += sectorsRead * 512;
            total.writeBytes += sectorsWritten * 512;
        }
    }
    return total;
}

std::vector<DiskInfo> getDiskPartitions() {
    std::vector<DiskInfo> disks;
    std::ifstream mounts("/proc/mounts");
    std::string line;
    
    while (getline(mounts, line)) {
        std::stringstream ss(line);
        std::string device, mountPoint, fsType;
        ss >> device >> mountPoint >> fsType;
        
        if (fsType == "ext4" || fsType == "btrfs" || fsType == "xfs" || 
            fsType == "ntfs" || fsType == "vfat" || fsType == "exfat") {
            
            struct statvfs stat;
            if (statvfs(mountPoint.c_str(), &stat) == 0) {
                DiskInfo info;
                info.name = mountPoint;
                if (info.name.length() > 15) info.name = "..." + info.name.substr(info.name.length() - 12);
                
                info.totalGB = (stat.f_blocks * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                float freeGB = (stat.f_bfree * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                info.usedGB = info.totalGB - freeGB;
                info.usagePercent = (info.totalGB > 0) ? (info.usedGB / info.totalGB) : 0;
                
                if (info.totalGB > 0.1) {
                    disks.push_back(info);
                }
            }
        }
    }
    return disks;
}

// ==================== GPU FONKSİYONLARI ====================
GPUInfo getGPUInfo() {
    GPUInfo gpu = {"", 0, 0, 0, 0, false};
    
    FILE* pipe = popen("nvidia-smi --query-gpu=name,temperature.gpu,utilization.gpu,memory.used,memory.total --format=csv,noheader,nounits 2>/dev/null", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            std::stringstream ss(buffer);
            std::string name, temp, usage, memUsed, memTotal;
            getline(ss, name, ',');
            getline(ss, temp, ',');
            getline(ss, usage, ',');
            getline(ss, memUsed, ',');
            getline(ss, memTotal, ',');
            
            gpu.name = name;
            if (gpu.name.length() > 25) gpu.name = gpu.name.substr(0, 22) + "...";
            try {
                gpu.temp = std::stoi(temp);
                gpu.usagePercent = std::stoi(usage);
                gpu.memUsedMB = std::stoi(memUsed);
                gpu.memTotalMB = std::stoi(memTotal);
                gpu.available = true;
            } catch (...) {}
        }
        pclose(pipe);
    }
    
    if (!gpu.available) {
        std::ifstream amdTemp("/sys/class/drm/card0/device/hwmon/hwmon0/temp1_input");
        if (amdTemp.is_open()) {
            int temp;
            amdTemp >> temp;
            gpu.temp = temp / 1000;
            gpu.name = "AMD GPU";
            gpu.available = true;
        }
    }
    
    if (!gpu.available) {
        std::ifstream intelGpu("/sys/class/drm/card0/device/vendor");
        if (intelGpu.is_open()) {
            std::string vendor;
            intelGpu >> vendor;
            if (vendor == "0x8086") {
                gpu.name = "Intel iGPU";
                gpu.available = true;
            }
        }
    }
    
    return gpu;
}

// ==================== PİL FONKSİYONLARI ====================
BatteryInfo getBatteryInfo() {
    BatteryInfo bat = {0, false, false, 0};
    
    const char* batPaths[] = {"/sys/class/power_supply/BAT0", "/sys/class/power_supply/BAT1"};
    
    for (const char* basePath : batPaths) {
        std::string capacityPath = std::string(basePath) + "/capacity";
        std::ifstream capFile(capacityPath);
        if (capFile.is_open()) {
            capFile >> bat.percent;
            bat.available = true;
            
            std::string statusPath = std::string(basePath) + "/status";
            std::ifstream statusFile(statusPath);
            if (statusFile.is_open()) {
                std::string status;
                statusFile >> status;
                bat.charging = (status == "Charging" || status == "Full");
            }
            
            std::string powerPath = std::string(basePath) + "/power_now";
            std::ifstream powerFile(powerPath);
            if (powerFile.is_open()) {
                long long power;
                powerFile >> power;
                bat.powerWatts = power / 1000000.0f;
            }
            break;
        }
    }
    return bat;
}

// ==================== SİSTEM BİLGİLERİ ====================
std::string getUptime() {
    std::ifstream file("/proc/uptime");
    double uptime;
    file >> uptime;
    
    int days = (int)(uptime / 86400);
    int hours = (int)((uptime - days * 86400) / 3600);
    int mins = (int)((uptime - days * 86400 - hours * 3600) / 60);
    
    std::stringstream ss;
    if (days > 0) ss << days << "d ";
    ss << hours << "h " << mins << "m";
    return ss.str();
}

int getProcessCount() {
    int count = 0;
    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
                count++;
            }
        }
        closedir(dir);
    }
    return count;
}

float getLoadAverage() {
    std::ifstream file("/proc/loadavg");
    float load1;
    file >> load1;
    return load1;
}

std::string formatSpeed(double bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes > 1024 * 1024) ss << (bytes / (1024.0 * 1024.0)) << " MB/s";
    else if (bytes > 1024) ss << (bytes / 1024.0) << " KB/s";
    else ss << bytes << " B/s";
    return ss.str();
}

// ==================== ANA PROGRAM ====================
int main(int, char**) {
    glfwSetErrorCallback([](int error, const char* description){ 
        fprintf(stderr, "GLFW Error %d: %s\n", error, description); 
    });
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1100, 750, "Eray Monitor v5.0", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    SetupModernStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Başlangıç değerleri
    CPUStats oldCpu = readCPUStats();
    NetStats oldNet = readNetStats();
    DiskStats oldDisk = readDiskStats();
    
    std::string cpuModel = getCPUModel();
    int cpuCores = getCPUCoreCount();
    float cpuMaxFreq = getCPUMaxFrequency();
    
    float cpuUsage = 0, cpuTemp = 0, cpuFreq = 0;
    float ramUsage = 0, ramTotal = 0, ramAvail = 0, swapTotal = 0, swapUsed = 0;
    double downSpeed = 0, upSpeed = 0;
    double diskReadSpeed = 0, diskWriteSpeed = 0;
    GPUInfo gpu = {};
    BatteryInfo battery = {};
    std::vector<DiskInfo> diskPartitions;
    int processCount = 0;
    float loadAvg = 0;
    
    std::vector<float> cpuHistory(60, 0), ramHistory(60, 0);
    std::vector<float> gpuHistory(60, 0);
    std::vector<float> netDownHistory(60, 0), netUpHistory(60, 0);
    
    double lastUpdateTime = 0;
    int slowUpdateCounter = 0;
    float animTime = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        animTime += io.DeltaTime;

        double currentTime = glfwGetTime();
        if (currentTime - lastUpdateTime >= 1.0) {
            CPUStats newCpu = readCPUStats();
            long long prevIdle = oldCpu.idle + oldCpu.iowait;
            long long idle = newCpu.idle + newCpu.iowait;
            long long totald = (idle + newCpu.user + newCpu.nice + newCpu.system + newCpu.irq + newCpu.softirq + newCpu.steal) - 
                               (prevIdle + oldCpu.user + oldCpu.nice + oldCpu.system + oldCpu.irq + oldCpu.softirq + oldCpu.steal);
            long long idled = idle - prevIdle;
            if (totald > 0) cpuUsage = (float)(totald - idled) / totald;
            oldCpu = newCpu;
            
            cpuTemp = getCPUTemp();
            cpuFreq = getCPUFrequency();
            ramUsage = getRamUsage(ramTotal, ramAvail, swapTotal, swapUsed);
            
            NetStats newNet = readNetStats();
            downSpeed = newNet.rxBytes - oldNet.rxBytes;
            upSpeed = newNet.txBytes - oldNet.txBytes;
            oldNet = newNet;
            
            DiskStats newDisk = readDiskStats();
            diskReadSpeed = newDisk.readBytes - oldDisk.readBytes;
            diskWriteSpeed = newDisk.writeBytes - oldDisk.writeBytes;
            oldDisk = newDisk;
            
            slowUpdateCounter++;
            if (slowUpdateCounter >= 5) {
                gpu = getGPUInfo();
                battery = getBatteryInfo();
                diskPartitions = getDiskPartitions();
                processCount = getProcessCount();
                loadAvg = getLoadAverage();
                slowUpdateCounter = 0;
            }
            
            cpuHistory.erase(cpuHistory.begin()); cpuHistory.push_back(cpuUsage * 100);
            ramHistory.erase(ramHistory.begin()); ramHistory.push_back(ramUsage * 100);
            gpuHistory.erase(gpuHistory.begin()); gpuHistory.push_back((float)gpu.usagePercent);
            netDownHistory.erase(netDownHistory.begin()); netDownHistory.push_back((float)(downSpeed / 1024));
            netUpHistory.erase(netUpHistory.begin()); netUpHistory.push_back((float)(upSpeed / 1024));
            
            lastUpdateTime = currentTime;
        }

        // ==================== MODERN ARAYÜZ ====================
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##Main", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ===== HEADER =====
        float headerHeight = 60;
        ImGui::BeginChild("Header", ImVec2(-1, headerHeight), false);
        {
            ImGui::SetCursorPosY(10);
            float pulse = 0.7f + 0.3f * sinf(animTime * 2.0f);
            ImVec4 titleColor = ImVec4(0.0f * pulse + 0.3f, 0.9f * pulse, 1.0f * pulse, 1.0f);
            
            ImGui::PushStyleColor(ImGuiCol_Text, titleColor);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Text("  ERAY MONITOR");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
            ImGui::TextColored(Colors::TextMuted, "v5.0");
            
            float rightOffset = ImGui::GetWindowWidth() - 350;
            ImGui::SameLine(rightOffset);
            ImGui::SetCursorPosY(15);
            
            ImGui::TextColored(Colors::TextSecondary, "Uptime");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Cyan, "%s", getUptime().c_str());
            
            ImGui::SameLine(rightOffset + 130);
            ImGui::TextColored(Colors::TextSecondary, "Procs");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Purple, "%d", processCount);
            
            ImGui::SameLine(rightOffset + 220);
            ImGui::TextColored(Colors::TextSecondary, "Load");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Orange, "%.2f", loadAvg);
        }
        ImGui::EndChild();
        
        ImGui::Spacing();

        // ===== İÇERİK ALANI =====
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float leftWidth = contentWidth * 0.52f;
        float rightWidth = contentWidth * 0.46f;
        float cardSpacing = 12.0f;

        // SOL PANEL
        ImGui::BeginChild("LeftPanel", ImVec2(leftWidth, -1), false);
        {
            // ===== CPU KARTI =====
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::BeginChild("CPUCard", ImVec2(-1, 200), true, ImGuiWindowFlags_NoScrollbar);
            {
                CardHeader("[CPU]", "PROCESSOR", Colors::Orange);
                
                ImGui::TextColored(Colors::TextPrimary, "%s", cpuModel.c_str());
                ImGui::TextColored(Colors::TextSecondary, "%d Cores  |  %.0f / %.0f MHz", cpuCores, cpuFreq, cpuMaxFreq);
                
                ImGui::Spacing();
                
                ImGui::Columns(2, NULL, false);
                ImGui::SetColumnWidth(0, 140);
                
                char cpuValueStr[16];
                snprintf(cpuValueStr, sizeof(cpuValueStr), "%.0f%%", cpuUsage * 100);
                DrawCircularProgress(cpuUsage, 50, Colors::Orange, "USAGE", cpuValueStr);
                
                ImGui::NextColumn();
                
                ImVec4 tempColor = (cpuTemp > 80) ? Colors::Red : (cpuTemp > 60) ? Colors::Yellow : Colors::Green;
                ImGui::TextColored(Colors::TextSecondary, "Temperature");
                ImGui::SameLine(120);
                ImGui::TextColored(tempColor, "%.0f C", cpuTemp);
                
                ImGui::Spacing();
                MiniGraph("##cpuGraph", cpuHistory.data(), (int)cpuHistory.size(), Colors::Orange, 50);
                
                ImGui::Columns(1);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            
            // ===== RAM KARTI =====
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::BeginChild("RAMCard", ImVec2(-1, 160), true, ImGuiWindowFlags_NoScrollbar);
            {
                CardHeader("[RAM]", "MEMORY", Colors::Green);
                
                float usedRam = ramTotal - ramAvail;
                char ramStr[64];
                snprintf(ramStr, sizeof(ramStr), "%.1f / %.1f GB  (%.0f%%)", usedRam, ramTotal, ramUsage * 100);
                ImGui::TextColored(Colors::TextPrimary, "%s", ramStr);
                
                ImGui::Spacing();
                char ramOverlay[16];
                snprintf(ramOverlay, sizeof(ramOverlay), "%.0f%%", ramUsage * 100);
                DrawGradientProgressBar(ramUsage, ImVec2(-1, 20), Colors::Green, Colors::Cyan, ramOverlay);
                
                ImGui::Spacing();
                MiniGraph("##ramGraph", ramHistory.data(), (int)ramHistory.size(), Colors::Green, 40);
                
                if (swapTotal > 0.01f) {
                    ImGui::Spacing();
                    float swapPercent = swapUsed / swapTotal;
                    ImGui::TextColored(Colors::TextSecondary, "Swap: %.1f / %.1f GB", swapUsed, swapTotal);
                    DrawGradientProgressBar(swapPercent, ImVec2(-1, 12), Colors::Purple, Colors::Pink, NULL);
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            
            // ===== GPU KARTI =====
            if (gpu.available) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("GPUCard", ImVec2(-1, 170), true, ImGuiWindowFlags_NoScrollbar);
                {
                    CardHeader("[GPU]", "GRAPHICS", Colors::Purple);
                    
                    ImGui::TextColored(Colors::TextPrimary, "%s", gpu.name.c_str());
                    
                    ImGui::Spacing();
                    ImGui::Columns(2, NULL, false);
                    
                    if (gpu.temp > 0) {
                        ImVec4 tempColor = (gpu.temp > 80) ? Colors::Red : Colors::Green;
                        ImGui::TextColored(Colors::TextSecondary, "Temp");
                        ImGui::SameLine(60);
                        ImGui::TextColored(tempColor, "%d C", gpu.temp);
                    }
                    
                    if (gpu.usagePercent >= 0) {
                        ImGui::TextColored(Colors::TextSecondary, "Usage");
                        ImGui::SameLine(60);
                        ImGui::TextColored(Colors::Purple, "%d%%", gpu.usagePercent);
                    }
                    
                    ImGui::NextColumn();
                    
                    if (gpu.memTotalMB > 0) {
                        ImGui::TextColored(Colors::TextSecondary, "VRAM");
                        ImGui::SameLine(60);
                        ImGui::TextColored(Colors::Cyan, "%d/%d MB", gpu.memUsedMB, gpu.memTotalMB);
                    }
                    
                    ImGui::Columns(1);
                    
                    ImGui::Spacing();
                    if (gpu.usagePercent >= 0) {
                        char gpuOverlay[16];
                        snprintf(gpuOverlay, sizeof(gpuOverlay), "%d%%", gpu.usagePercent);
                        DrawGradientProgressBar(gpu.usagePercent / 100.0f, ImVec2(-1, 16), Colors::Purple, Colors::Pink, gpuOverlay);
                    }
                    
                    if (gpu.memTotalMB > 0) {
                        ImGui::Spacing();
                        DrawGradientProgressBar(gpu.memUsedMB / (float)gpu.memTotalMB, ImVec2(-1, 12), Colors::Blue, Colors::Cyan, NULL);
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndChild();
        
        ImGui::SameLine(0, cardSpacing);
        
        // SAĞ PANEL
        ImGui::BeginChild("RightPanel", ImVec2(rightWidth, -1), false);
        {
            // ===== NETWORK KARTI =====
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::BeginChild("NetCard", ImVec2(-1, 180), true, ImGuiWindowFlags_NoScrollbar);
            {
                CardHeader("[NET]", "NETWORK", Colors::Cyan);
                
                ImGui::Columns(2, NULL, false);
                
                ImGui::TextColored(Colors::TextSecondary, "Download");
                ImGui::TextColored(Colors::Cyan, "%s", formatSpeed(downSpeed).c_str());
                MiniGraph("##netDown", netDownHistory.data(), (int)netDownHistory.size(), Colors::Cyan, 35);
                
                ImGui::NextColumn();
                
                ImGui::TextColored(Colors::TextSecondary, "Upload");
                ImGui::TextColored(Colors::Pink, "%s", formatSpeed(upSpeed).c_str());
                MiniGraph("##netUp", netUpHistory.data(), (int)netUpHistory.size(), Colors::Pink, 35);
                
                ImGui::Columns(1);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::TextColored(Colors::TextSecondary, "Disk I/O:");
                ImGui::SameLine();
                ImGui::TextColored(Colors::Green, "R: %s", formatSpeed(diskReadSpeed).c_str());
                ImGui::SameLine();
                ImGui::TextColored(Colors::Orange, "W: %s", formatSpeed(diskWriteSpeed).c_str());
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            
            // ===== DISK KARTI =====
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            float diskCardHeight = 60.0f + diskPartitions.size() * 45.0f;
            if (diskCardHeight > 200) diskCardHeight = 200;
            ImGui::BeginChild("DiskCard", ImVec2(-1, diskCardHeight), true);
            {
                CardHeader("[HDD]", "STORAGE", Colors::Yellow);
                
                for (const auto& disk : diskPartitions) {
                    ImGui::TextColored(Colors::TextPrimary, "%s", disk.name.c_str());
                    ImGui::SameLine(100);
                    ImGui::TextColored(Colors::TextSecondary, "%.1f / %.1f GB", disk.usedGB, disk.totalGB);
                    
                    ImVec4 diskColor = (disk.usagePercent > 0.9f) ? Colors::Red : 
                                       (disk.usagePercent > 0.7f) ? Colors::Yellow : Colors::Green;
                    DrawGradientProgressBar(disk.usagePercent, ImVec2(-1, 14), diskColor, diskColor, NULL);
                    ImGui::Spacing();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            
            // ===== PİL KARTI =====
            if (battery.available) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("BatteryCard", ImVec2(-1, 100), true, ImGuiWindowFlags_NoScrollbar);
                {
                    CardHeader("[BAT]", "BATTERY", Colors::Green);
                    
                    ImVec4 batColor = (battery.percent < 20) ? Colors::Red : 
                                      (battery.percent < 50) ? Colors::Yellow : Colors::Green;
                    
                    const char* status = battery.charging ? "Charging" : "On Battery";
                    ImGui::TextColored(batColor, "%d%%", battery.percent);
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::TextSecondary, "%s", status);
                    
                    if (battery.powerWatts > 0) {
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60);
                        ImGui::TextColored(Colors::Orange, "%.1f W", battery.powerWatts);
                    }
                    
                    ImGui::Spacing();
                    char batOverlay[16];
                    snprintf(batOverlay, sizeof(batOverlay), "%d%%", battery.percent);
                    DrawGradientProgressBar(battery.percent / 100.0f, ImVec2(-1, 18), batColor, Colors::Green, batOverlay);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndChild();
        
        ImGui::End();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.06f, 0.06f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

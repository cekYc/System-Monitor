// CekyMonitor v6.0 - Premium Linux System Monitor
// Features: CPU (per-core), RAM (detailed), GPU, Disk, Network, Battery, Frequency,
//           Top Processes, System Info, Thread Count, Context Switches, Interrupts,
//           Load Averages, IO Wait, Kernel/Distro Info, Disk I/O, File Descriptors
// UI: Modern Glassmorphism, Neon Accents, Smooth Animations, Card Layout, Tab Navigation

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
#include <algorithm>
#include <numeric>
#include <dirent.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <cmath>
#include <pwd.h>
#include <GLFW/glfw3.h>

// ==================== DATA STRUCTURES ====================
struct CPUStats { long long user, nice, system, idle, iowait, irq, softirq, steal; };
struct NetStats { long long rxBytes, txBytes; };
struct DiskStats { long long readBytes, writeBytes; };
struct DiskInfo { std::string name; std::string fsType; float totalGB, usedGB, usagePercent; };
struct GPUInfo { std::string name; int temp; int usagePercent; int memUsedMB, memTotalMB; int fanSpeed; int powerDraw; bool available; };
struct BatteryInfo { int percent; bool charging; bool available; float powerWatts; int estimatedMinutes; };
struct DiskIOInfo { double readBytesPerSec, writeBytesPerSec; long long totalRead, totalWrite; };

struct ProcessInfo {
    std::string name;
    int pid;
    float cpuPercent;
    float memMB;
    int threadCount;
    char state;
};

struct SystemInfo {
    std::string distroName, distroVersion, kernelVersion;
    std::string hostname, username, architecture;
    int logicalCores, physicalCores;
    float totalRamGB;
};

struct MemoryDetail {
    long long totalKB, availableKB, freeKB;
    long long buffersKB, cachedKB, sreclaimableKB;
    long long dirtyKB, writebackKB;
    long long swapTotalKB, swapFreeKB, swapCachedKB;
    long long sharedKB, activeKB, inactiveKB;
};

// ==================== COLOR PALETTE ====================
namespace Colors {
    const ImVec4 Background   = ImVec4(0.05f, 0.05f, 0.08f, 1.0f);
    const ImVec4 CardBg       = ImVec4(0.09f, 0.09f, 0.12f, 0.97f);
    const ImVec4 CardBgHover  = ImVec4(0.12f, 0.12f, 0.16f, 0.97f);
    const ImVec4 CardBorder   = ImVec4(0.15f, 0.15f, 0.20f, 0.6f);
    const ImVec4 HeaderBg     = ImVec4(0.07f, 0.07f, 0.10f, 0.98f);

    const ImVec4 Cyan         = ImVec4(0.0f, 0.85f, 1.0f, 1.0f);
    const ImVec4 CyanDark     = ImVec4(0.0f, 0.45f, 0.55f, 1.0f);
    const ImVec4 Purple       = ImVec4(0.65f, 0.3f, 1.0f, 1.0f);
    const ImVec4 PurpleDark   = ImVec4(0.35f, 0.15f, 0.55f, 1.0f);
    const ImVec4 Pink         = ImVec4(1.0f, 0.35f, 0.65f, 1.0f);
    const ImVec4 Green        = ImVec4(0.15f, 0.95f, 0.5f, 1.0f);
    const ImVec4 GreenDark    = ImVec4(0.08f, 0.5f, 0.25f, 1.0f);
    const ImVec4 Orange       = ImVec4(1.0f, 0.55f, 0.15f, 1.0f);
    const ImVec4 OrangeDark   = ImVec4(0.55f, 0.3f, 0.08f, 1.0f);
    const ImVec4 Yellow       = ImVec4(1.0f, 0.88f, 0.25f, 1.0f);
    const ImVec4 Red          = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
    const ImVec4 RedDark      = ImVec4(0.55f, 0.12f, 0.12f, 1.0f);
    const ImVec4 Blue         = ImVec4(0.25f, 0.5f, 1.0f, 1.0f);
    const ImVec4 BlueDark     = ImVec4(0.12f, 0.25f, 0.55f, 1.0f);
    const ImVec4 Teal         = ImVec4(0.0f, 0.8f, 0.7f, 1.0f);
    const ImVec4 Amber        = ImVec4(1.0f, 0.75f, 0.0f, 1.0f);

    const ImVec4 TextPrimary   = ImVec4(0.94f, 0.94f, 0.97f, 1.0f);
    const ImVec4 TextSecondary = ImVec4(0.55f, 0.55f, 0.65f, 1.0f);
    const ImVec4 TextMuted     = ImVec4(0.35f, 0.35f, 0.45f, 1.0f);
    const ImVec4 TextHighlight = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

// ==================== STYLE SETUP ====================
void SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 14.0f;
    style.FrameRounding     = 10.0f;
    style.GrabRounding      = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabRounding       = 8.0f;
    style.ChildRounding     = 12.0f;
    style.PopupRounding     = 10.0f;

    style.WindowPadding   = ImVec2(14, 14);
    style.FramePadding    = ImVec2(12, 6);
    style.ItemSpacing     = ImVec2(10, 6);
    style.ItemInnerSpacing = ImVec2(8, 5);
    style.IndentSpacing   = 20.0f;

    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize  = 0.0f;
    style.PopupBorderSize  = 1.0f;
    style.ChildBorderSize  = 1.0f;

    style.ScrollbarSize = 10.0f;
    style.GrabMinSize   = 10.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]    = Colors::Background;
    colors[ImGuiCol_ChildBg]     = Colors::CardBg;
    colors[ImGuiCol_PopupBg]     = ImVec4(0.08f, 0.08f, 0.11f, 0.98f);
    colors[ImGuiCol_Border]      = Colors::CardBorder;
    colors[ImGuiCol_BorderShadow]= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_Text]        = Colors::TextPrimary;
    colors[ImGuiCol_TextDisabled]= Colors::TextMuted;

    colors[ImGuiCol_FrameBg]        = ImVec4(0.12f, 0.12f, 0.16f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);

    colors[ImGuiCol_TitleBg]          = Colors::CardBg;
    colors[ImGuiCol_TitleBgActive]    = Colors::CardBg;
    colors[ImGuiCol_TitleBgCollapsed] = Colors::CardBg;

    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.06f, 0.06f, 0.08f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);

    colors[ImGuiCol_Button]        = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
    colors[ImGuiCol_ButtonActive]  = ImVec4(0.28f, 0.28f, 0.35f, 1.0f);

    colors[ImGuiCol_Header]        = ImVec4(0.15f, 0.15f, 0.20f, 0.6f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.26f, 0.8f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);

    colors[ImGuiCol_Separator]        = ImVec4(0.18f, 0.18f, 0.22f, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = Colors::Cyan;
    colors[ImGuiCol_SeparatorActive]  = Colors::Cyan;

    colors[ImGuiCol_Tab]     = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.26f, 1.0f);
    colors[ImGuiCol_TabActive]  = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);

    colors[ImGuiCol_PlotLines]            = Colors::Cyan;
    colors[ImGuiCol_PlotLinesHovered]     = Colors::Pink;
    colors[ImGuiCol_PlotHistogram]        = Colors::Purple;
    colors[ImGuiCol_PlotHistogramHovered] = Colors::Pink;

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.18f, 0.24f, 1.0f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.14f, 0.14f, 0.18f, 1.0f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.08f, 0.08f, 0.10f, 0.4f);
}

// ==================== CUSTOM DRAWING ====================

ImVec4 GetUsageColor(float percent) {
    if (percent > 90.0f) return Colors::Red;
    if (percent > 75.0f) return Colors::Orange;
    if (percent > 50.0f) return Colors::Yellow;
    return Colors::Green;
}

void DrawGradientProgressBar(float fraction, ImVec2 size, ImVec4 colorLeft, ImVec4 colorRight, const char* overlay = nullptr) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImVec2 pos = window->DC.CursorPos;
    if (size.x < 0) size.x = ImGui::GetContentRegionAvail().x;
    ImVec2 endPos = ImVec2(pos.x + size.x, pos.y + size.y);

    window->DrawList->AddRectFilled(pos, endPos, IM_COL32(22, 22, 28, 255), 6.0f);

    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        ImVec2 fillEnd = ImVec2(pos.x + size.x * fraction, endPos.y);
        ImU32 colL = ImGui::ColorConvertFloat4ToU32(colorLeft);
        ImU32 colR = ImGui::ColorConvertFloat4ToU32(colorRight);
        window->DrawList->AddRectFilledMultiColor(pos, fillEnd, colL, colR, colR, colL);
        // subtle glow
        ImU32 glow = IM_COL32((int)(colorLeft.x*255), (int)(colorLeft.y*255), (int)(colorLeft.z*255), 30);
        window->DrawList->AddRectFilled(ImVec2(pos.x, pos.y - 1), ImVec2(fillEnd.x, endPos.y + 1), glow, 6.0f);
    }

    if (overlay) {
        ImVec2 ts = ImGui::CalcTextSize(overlay);
        ImVec2 tp = ImVec2(pos.x + (size.x - ts.x) * 0.5f, pos.y + (size.y - ts.y) * 0.5f);
        window->DrawList->AddText(tp, IM_COL32(255, 255, 255, 240), overlay);
    }

    ImGui::ItemSize(size);
}

void DrawCircularProgress(float fraction, float radius, ImVec4 color, const char* label, const char* value) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);

    window->DrawList->AddCircle(center, radius, IM_COL32(35, 35, 45, 255), 36, 4.0f);

    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        float startAngle = -3.14159f / 2.0f;
        float endAngle = startAngle + fraction * 2.0f * 3.14159f;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(color);

        int segments = (int)(fraction * 36);
        if (segments < 3) segments = 3;

        for (int i = 0; i < segments; i++) {
            float a1 = startAngle + (endAngle - startAngle) * (float)i / segments;
            float a2 = startAngle + (endAngle - startAngle) * (float)(i + 1) / segments;
            ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
            ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
            window->DrawList->AddLine(p1, p2, col, 6.0f);
        }
    }

    ImVec2 vs = ImGui::CalcTextSize(value);
    window->DrawList->AddText(ImVec2(center.x - vs.x * 0.5f, center.y - 10), IM_COL32(255, 255, 255, 255), value);
    ImVec2 ls = ImGui::CalcTextSize(label);
    window->DrawList->AddText(ImVec2(center.x - ls.x * 0.5f, center.y + 8), IM_COL32(140, 140, 155, 255), label);

    ImGui::ItemSize(ImVec2(radius * 2, radius * 2));
}

void DrawCardBorderGlow(ImVec4 color, float alpha = 0.35f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 mn = window->ContentRegionRect.Min - ImVec2(2, 2);
    ImVec2 mx = window->ContentRegionRect.Max + ImVec2(2, 2);
    ImU32 c = IM_COL32((int)(color.x*255), (int)(color.y*255), (int)(color.z*255), (int)(alpha*255));
    window->DrawList->AddRect(mn, mx, c, 14.0f, 0, 1.5f);
}

void CardHeader(const char* icon, const char* title, ImVec4 color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%s", icon);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", title);
    ImGui::Spacing();
}

void MiniGraph(const char* id, float* data, int count, ImVec4 color, float height = 40.0f) {
    ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.10f, 0.8f));
    ImGui::PlotLines(id, data, count, 0, NULL, 0, 100, ImVec2(-1, height));
    ImGui::PopStyleColor(2);
}

void DrawStatRow(const char* label, const char* value, ImVec4 valueColor = Colors::TextPrimary) {
    ImGui::TextColored(Colors::TextSecondary, "%s", label);
    ImGui::SameLine(180);
    ImGui::TextColored(valueColor, "%s", value);
}

// ==================== CPU FUNCTIONS ====================
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

std::vector<CPUStats> readPerCoreCPUStats() {
    std::vector<CPUStats> cores;
    std::ifstream file("/proc/stat");
    std::string line;
    getline(file, line); // skip total
    while (getline(file, line)) {
        if (line.substr(0, 3) != "cpu") break;
        CPUStats s = {0};
        std::stringstream ss(line);
        std::string label;
        ss >> label >> s.user >> s.nice >> s.system >> s.idle
           >> s.iowait >> s.irq >> s.softirq >> s.steal;
        cores.push_back(s);
    }
    return cores;
}

float calcCPUUsage(const CPUStats& oldS, const CPUStats& newS) {
    long long prevIdle = oldS.idle + oldS.iowait;
    long long idle = newS.idle + newS.iowait;
    long long prevTotal = prevIdle + oldS.user + oldS.nice + oldS.system + oldS.irq + oldS.softirq + oldS.steal;
    long long total = idle + newS.user + newS.nice + newS.system + newS.irq + newS.softirq + newS.steal;
    long long totald = total - prevTotal;
    long long idled = idle - prevIdle;
    if (totald <= 0) return 0.0f;
    return (float)(totald - idled) / (float)totald;
}

float getCPUTemp() {
    const char* paths[] = {
        "/sys/class/thermal/thermal_zone0/temp",
        "/sys/class/hwmon/hwmon0/temp1_input",
        "/sys/class/hwmon/hwmon1/temp1_input",
        "/sys/class/hwmon/hwmon2/temp1_input",
        "/sys/class/hwmon/hwmon3/temp1_input"
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
        return freq / 1000.0f; // MHz
    }
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) return std::stof(line.substr(pos + 1));
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

int getPhysicalCoreCount() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    std::vector<std::string> coreIds;
    while (getline(file, line)) {
        if (line.find("core id") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string id = line.substr(pos + 1);
                // trim
                id.erase(0, id.find_first_not_of(" \t"));
                id.erase(id.find_last_not_of(" \t") + 1);
                if (std::find(coreIds.begin(), coreIds.end(), id) == coreIds.end())
                    coreIds.push_back(id);
            }
        }
    }
    return coreIds.empty() ? 1 : (int)coreIds.size();
}

std::string getCPUModel() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    while (getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 2);
                if (model.length() > 40) model = model.substr(0, 37) + "...";
                return model;
            }
        }
    }
    return "Unknown CPU";
}

void getLoadAverages(float& l1, float& l5, float& l15) {
    std::ifstream file("/proc/loadavg");
    file >> l1 >> l5 >> l15;
}

void getContextSwitchesAndInterrupts(long long& ctxt, long long& intr) {
    std::ifstream file("/proc/stat");
    std::string line;
    ctxt = 0; intr = 0;
    while (getline(file, line)) {
        if (line.substr(0, 4) == "ctxt") {
            std::stringstream ss(line);
            std::string label;
            ss >> label >> ctxt;
        } else if (line.substr(0, 4) == "intr") {
            std::stringstream ss(line);
            std::string label;
            ss >> label >> intr;
        }
    }
}

// ==================== MEMORY FUNCTIONS ====================
MemoryDetail getMemoryDetail() {
    MemoryDetail m = {0};
    std::ifstream file("/proc/meminfo");
    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string label;
        long long value;
        ss >> label >> value;
        if (label == "MemTotal:")       m.totalKB = value;
        else if (label == "MemAvailable:") m.availableKB = value;
        else if (label == "MemFree:")    m.freeKB = value;
        else if (label == "Buffers:")    m.buffersKB = value;
        else if (label == "Cached:")     m.cachedKB = value;
        else if (label == "SReclaimable:") m.sreclaimableKB = value;
        else if (label == "Dirty:")      m.dirtyKB = value;
        else if (label == "Writeback:")  m.writebackKB = value;
        else if (label == "SwapTotal:")  m.swapTotalKB = value;
        else if (label == "SwapFree:")   m.swapFreeKB = value;
        else if (label == "SwapCached:") m.swapCachedKB = value;
        else if (label == "Shmem:")      m.sharedKB = value;
        else if (label == "Active:")     m.activeKB = value;
        else if (label == "Inactive:")   m.inactiveKB = value;
    }
    return m;
}

// ==================== NETWORK FUNCTIONS ====================
NetStats readNetStats() {
    std::ifstream file("/proc/net/dev");
    std::string line;
    NetStats total = {0, 0};
    getline(file, line); getline(file, line); // skip headers
    while (getline(file, line)) {
        if (line.find("lo:") != std::string::npos) continue;
        size_t colon = line.find(':');
        if (colon != std::string::npos) line[colon] = ' ';
        std::stringstream ss(line);
        std::string iface;
        long long rx = 0, tx = 0, junk = 0;
        ss >> iface >> rx >> junk >> junk >> junk >> junk >> junk >> junk >> junk >> tx;
        if (!ss.fail()) { total.rxBytes += rx; total.txBytes += tx; }
    }
    return total;
}

int getNetworkConnectionCount() {
    int count = 0;
    std::ifstream tcp("/proc/net/tcp");
    std::string line;
    if (tcp.is_open()) {
        getline(tcp, line); // header
        while (getline(tcp, line)) count++;
    }
    std::ifstream tcp6("/proc/net/tcp6");
    if (tcp6.is_open()) {
        getline(tcp6, line);
        while (getline(tcp6, line)) count++;
    }
    return count;
}

// ==================== DISK FUNCTIONS ====================
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
        if (fsType == "ext4" || fsType == "btrfs" || fsType == "xfs" || fsType == "zfs" ||
            fsType == "ntfs" || fsType == "vfat" || fsType == "exfat" || fsType == "f2fs" || fsType == "tmpfs") {
            if (fsType == "tmpfs" && mountPoint != "/tmp") continue; // only show /tmp for tmpfs
            struct statvfs stat;
            if (statvfs(mountPoint.c_str(), &stat) == 0) {
                DiskInfo info;
                info.name = mountPoint;
                info.fsType = fsType;
                if (info.name.length() > 18) info.name = "..." + info.name.substr(info.name.length() - 15);
                info.totalGB = (stat.f_blocks * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                float freeGB = (stat.f_bfree * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                info.usedGB = info.totalGB - freeGB;
                info.usagePercent = (info.totalGB > 0) ? (info.usedGB / info.totalGB) : 0;
                if (info.totalGB > 0.1) disks.push_back(info);
            }
        }
    }
    return disks;
}

// ==================== GPU FUNCTIONS ====================
GPUInfo getGPUInfo() {
    GPUInfo gpu = {"", 0, 0, 0, 0, 0, 0, false};

    // NVIDIA via nvidia-smi
    FILE* pipe = popen("nvidia-smi --query-gpu=name,temperature.gpu,utilization.gpu,memory.used,memory.total,fan.speed,power.draw --format=csv,noheader,nounits 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            std::stringstream ss(buffer);
            std::string name, temp, usage, memUsed, memTotal, fan, power;
            getline(ss, name, ',');
            getline(ss, temp, ',');
            getline(ss, usage, ',');
            getline(ss, memUsed, ',');
            getline(ss, memTotal, ',');
            getline(ss, fan, ',');
            getline(ss, power, ',');
            gpu.name = name;
            if (gpu.name.length() > 28) gpu.name = gpu.name.substr(0, 25) + "...";
            try {
                gpu.temp = std::stoi(temp);
                gpu.usagePercent = std::stoi(usage);
                gpu.memUsedMB = std::stoi(memUsed);
                gpu.memTotalMB = std::stoi(memTotal);
                gpu.fanSpeed = std::stoi(fan);
                gpu.powerDraw = (int)std::stof(power);
                gpu.available = true;
            } catch (...) {}
        }
        pclose(pipe);
    }

    // AMD fallback
    if (!gpu.available) {
        const char* amdPaths[] = {
            "/sys/class/drm/card0/device/hwmon/hwmon0/temp1_input",
            "/sys/class/drm/card0/device/hwmon/hwmon1/temp1_input",
            "/sys/class/drm/card1/device/hwmon/hwmon0/temp1_input"
        };
        for (const char* p : amdPaths) {
            std::ifstream f(p);
            if (f.is_open()) {
                int t; f >> t;
                gpu.temp = t / 1000;
                gpu.name = "AMD GPU";
                gpu.available = true;
                // try to get usage
                std::ifstream busy("/sys/class/drm/card0/device/gpu_busy_percent");
                if (busy.is_open()) busy >> gpu.usagePercent;
                break;
            }
        }
    }

    // Intel iGPU fallback
    if (!gpu.available) {
        std::ifstream f("/sys/class/drm/card0/device/vendor");
        if (f.is_open()) {
            std::string v; f >> v;
            if (v == "0x8086") { gpu.name = "Intel iGPU"; gpu.available = true; }
        }
    }

    return gpu;
}

// ==================== BATTERY FUNCTIONS ====================
BatteryInfo getBatteryInfo() {
    BatteryInfo bat = {0, false, false, 0, 0};
    const char* paths[] = {"/sys/class/power_supply/BAT0", "/sys/class/power_supply/BAT1"};
    for (const char* base : paths) {
        std::string capPath = std::string(base) + "/capacity";
        std::ifstream capFile(capPath);
        if (capFile.is_open()) {
            capFile >> bat.percent;
            bat.available = true;

            std::ifstream statusFile(std::string(base) + "/status");
            if (statusFile.is_open()) {
                std::string status; statusFile >> status;
                bat.charging = (status == "Charging" || status == "Full");
            }

            std::ifstream powerFile(std::string(base) + "/power_now");
            if (powerFile.is_open()) {
                long long power; powerFile >> power;
                bat.powerWatts = power / 1000000.0f;
            }

            // estimated time
            std::ifstream energyNow(std::string(base) + "/energy_now");
            std::ifstream energyFull(std::string(base) + "/energy_full");
            if (energyNow.is_open() && energyFull.is_open() && bat.powerWatts > 0.1f) {
                long long eNow, eFull;
                energyNow >> eNow;
                energyFull >> eFull;
                if (!bat.charging)
                    bat.estimatedMinutes = (int)((eNow / 1000000.0f) / bat.powerWatts * 60);
                else
                    bat.estimatedMinutes = (int)(((eFull - eNow) / 1000000.0f) / bat.powerWatts * 60);
            }
            break;
        }
    }
    return bat;
}

// ==================== PROCESS FUNCTIONS ====================
struct ProcCPUTick { int pid; long long utime, stime; };

std::vector<ProcCPUTick> readAllProcTicks() {
    std::vector<ProcCPUTick> ticks;
    DIR* dir = opendir("/proc");
    if (!dir) return ticks;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') continue;
        int pid = atoi(entry->d_name);
        std::string statPath = std::string("/proc/") + entry->d_name + "/stat";
        std::ifstream f(statPath);
        if (!f.is_open()) continue;
        std::string line; getline(f, line);
        // find closing paren of comm
        size_t rp = line.rfind(')');
        if (rp == std::string::npos) continue;
        std::stringstream ss(line.substr(rp + 2));
        char state;
        long long ppid, pgrp, session, tty, tpgid, flags;
        long long minflt, cminflt, majflt, cmajflt, utime, stime;
        ss >> state >> ppid >> pgrp >> session >> tty >> tpgid >> flags
           >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;
        ticks.push_back({pid, utime, stime});
    }
    closedir(dir);
    return ticks;
}

std::vector<ProcessInfo> getTopProcesses(const std::vector<ProcCPUTick>& oldTicks, const std::vector<ProcCPUTick>& newTicks, long long totalCpuDelta, int maxCount = 20) {
    std::vector<ProcessInfo> procs;
    long long clkTck = sysconf(_SC_CLK_TCK);
    long long pageSize = sysconf(_SC_PAGESIZE);

    for (const auto& nt : newTicks) {
        float cpuPct = 0;
        for (const auto& ot : oldTicks) {
            if (ot.pid == nt.pid) {
                long long delta = (nt.utime + nt.stime) - (ot.utime + ot.stime);
                if (totalCpuDelta > 0) cpuPct = (float)delta / (float)totalCpuDelta * 100.0f;
                break;
            }
        }

        std::string statPath = "/proc/" + std::to_string(nt.pid) + "/stat";
        std::ifstream f(statPath);
        if (!f.is_open()) continue;
        std::string line; getline(f, line);
        // extract name
        size_t lp = line.find('(');
        size_t rp = line.rfind(')');
        if (lp == std::string::npos || rp == std::string::npos) continue;
        std::string name = line.substr(lp + 1, rp - lp - 1);
        if (name.length() > 20) name = name.substr(0, 17) + "...";

        // parse rest of fields for state, threads, rss
        std::stringstream ss(line.substr(rp + 2));
        char state;
        long long ppid, pgrp, session, tty, tpgid, flags;
        long long minflt, cminflt, majflt, cmajflt, utime2, stime2;
        long long cutime, cstime, priority, nice, numThreads, itrealvalue, starttime;
        long long vsize, rss;
        ss >> state >> ppid >> pgrp >> session >> tty >> tpgid >> flags
           >> minflt >> cminflt >> majflt >> cmajflt >> utime2 >> stime2
           >> cutime >> cstime >> priority >> nice >> numThreads >> itrealvalue >> starttime
           >> vsize >> rss;

        float memMB = (rss * pageSize) / (1024.0f * 1024.0f);

        ProcessInfo pi;
        pi.name = name;
        pi.pid = nt.pid;
        pi.cpuPercent = cpuPct;
        pi.memMB = memMB;
        pi.threadCount = (int)numThreads;
        pi.state = state;
        procs.push_back(pi);
    }

    // sort by CPU desc
    std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.cpuPercent > b.cpuPercent;
    });

    if ((int)procs.size() > maxCount) procs.resize(maxCount);
    return procs;
}

// ==================== SYSTEM INFO ====================
SystemInfo getSystemInfo() {
    SystemInfo info;

    // Distro from /etc/os-release
    std::ifstream osrel("/etc/os-release");
    std::string line;
    while (getline(osrel, line)) {
        if (line.substr(0, 12) == "PRETTY_NAME=") {
            info.distroName = line.substr(13);
            if (!info.distroName.empty() && info.distroName.back() == '"')
                info.distroName.pop_back();
        } else if (line.substr(0, 11) == "VERSION_ID=") {
            info.distroVersion = line.substr(12);
            if (!info.distroVersion.empty() && info.distroVersion.back() == '"')
                info.distroVersion.pop_back();
        }
    }

    // Kernel
    struct utsname uts;
    if (uname(&uts) == 0) {
        info.kernelVersion = uts.release;
        info.architecture = uts.machine;
    }

    // Hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
        info.hostname = hostname;

    // Username
    struct passwd* pw = getpwuid(getuid());
    if (pw) info.username = pw->pw_name;
    else {
        char* user = getenv("USER");
        info.username = user ? user : "unknown";
    }

    info.logicalCores = getCPUCoreCount();
    info.physicalCores = getPhysicalCoreCount();

    // Total RAM
    std::ifstream meminfo("/proc/meminfo");
    while (getline(meminfo, line)) {
        if (line.substr(0, 9) == "MemTotal:") {
            std::stringstream ss(line);
            std::string label; long long val;
            ss >> label >> val;
            info.totalRamGB = val / (1024.0f * 1024.0f);
            break;
        }
    }

    return info;
}

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
            if (entry->d_type == DT_DIR && entry->d_name[0] >= '0' && entry->d_name[0] <= '9')
                count++;
        }
        closedir(dir);
    }
    return count;
}

int getTotalThreadCount() {
    int total = 0;
    DIR* dir = opendir("/proc");
    if (!dir) return 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') continue;
        std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
        std::ifstream f(statusPath);
        std::string line;
        while (getline(f, line)) {
            if (line.substr(0, 8) == "Threads:") {
                std::stringstream ss(line);
                std::string label; int t;
                ss >> label >> t;
                total += t;
                break;
            }
        }
    }
    closedir(dir);
    return total;
}

int getFileDescriptorCount() {
    std::ifstream file("/proc/sys/fs/file-nr");
    int allocated = 0;
    if (file.is_open()) file >> allocated;
    return allocated;
}

std::string formatSpeed(double bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes > 1024 * 1024) ss << (bytes / (1024.0 * 1024.0)) << " MB/s";
    else if (bytes > 1024) ss << (bytes / 1024.0) << " KB/s";
    else ss << bytes << " B/s";
    return ss.str();
}

std::string formatBytes(long long bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes > 1073741824LL) ss << (bytes / 1073741824.0) << " GB";
    else if (bytes > 1048576LL) ss << (bytes / 1048576.0) << " MB";
    else if (bytes > 1024LL) ss << (bytes / 1024.0) << " KB";
    else ss << bytes << " B";
    return ss.str();
}

// ==================== MAIN ====================
int main(int, char**) {
    glfwSetErrorCallback([](int error, const char* description){
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    });
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 820, "Ceky Monitor v6.0", NULL, NULL);
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

    // ===== INITIAL STATE =====
    CPUStats oldCpu = readCPUStats();
    std::vector<CPUStats> oldCoreCpu = readPerCoreCPUStats();
    NetStats oldNet = readNetStats();
    DiskStats oldDisk = readDiskStats();
    std::vector<ProcCPUTick> oldProcTicks = readAllProcTicks();

    std::string cpuModel = getCPUModel();
    int cpuCores = getCPUCoreCount();
    int physCores = getPhysicalCoreCount();
    float cpuMaxFreq = getCPUMaxFrequency();
    SystemInfo sysInfo = getSystemInfo();

    float cpuUsage = 0, cpuTemp = 0, cpuFreq = 0;
    float peakCpu = 0;
    std::vector<float> perCoreUsage;
    float loadAvg1 = 0, loadAvg5 = 0, loadAvg15 = 0;
    long long contextSwitches = 0, interrupts = 0;
    long long prevCtxt = 0, prevIntr = 0;
    float ctxtPerSec = 0, intrPerSec = 0;

    MemoryDetail memDetail = {};
    float ramUsagePercent = 0;
    float peakRam = 0;

    double downSpeed = 0, upSpeed = 0;
    long long sessionRx = 0, sessionTx = 0;
    long long startRx = oldNet.rxBytes, startTx = oldNet.txBytes;
    int netConnections = 0;

    double diskReadSpeed = 0, diskWriteSpeed = 0;
    long long startDiskRead = oldDisk.readBytes, startDiskWrite = oldDisk.writeBytes;

    GPUInfo gpu = {};
    BatteryInfo battery = {};
    std::vector<DiskInfo> diskPartitions;
    std::vector<ProcessInfo> topProcesses;
    int processCount = 0;
    int threadCount = 0;
    int fdCount = 0;

    const int HISTORY_SIZE = 120;
    std::vector<float> cpuHistory(HISTORY_SIZE, 0), ramHistory(HISTORY_SIZE, 0);
    std::vector<float> gpuHistory(HISTORY_SIZE, 0);
    std::vector<float> netDownHistory(HISTORY_SIZE, 0), netUpHistory(HISTORY_SIZE, 0);
    std::vector<float> ioWaitHistory(HISTORY_SIZE, 0);

    double lastUpdateTime = 0;
    int slowUpdateCounter = 0;
    float animTime = 0;
    int currentTab = 0; // 0=Overview, 1=Processes, 2=System Info

    getContextSwitchesAndInterrupts(prevCtxt, prevIntr);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        animTime += io.DeltaTime;

        double currentTime = glfwGetTime();
        if (currentTime - lastUpdateTime >= 1.0) {
            // CPU
            CPUStats newCpu = readCPUStats();
            cpuUsage = calcCPUUsage(oldCpu, newCpu);
            if (cpuUsage * 100 > peakCpu) peakCpu = cpuUsage * 100;

            // IO Wait percentage
            long long totalDelta = (newCpu.user + newCpu.nice + newCpu.system + newCpu.idle + newCpu.iowait + newCpu.irq + newCpu.softirq + newCpu.steal)
                                 - (oldCpu.user + oldCpu.nice + oldCpu.system + oldCpu.idle + oldCpu.iowait + oldCpu.irq + oldCpu.softirq + oldCpu.steal);
            float ioWaitPct = totalDelta > 0 ? (float)(newCpu.iowait - oldCpu.iowait) / (float)totalDelta * 100.0f : 0;

            // Per-core
            std::vector<CPUStats> newCoreCpu = readPerCoreCPUStats();
            perCoreUsage.resize(newCoreCpu.size());
            for (size_t i = 0; i < newCoreCpu.size() && i < oldCoreCpu.size(); i++) {
                perCoreUsage[i] = calcCPUUsage(oldCoreCpu[i], newCoreCpu[i]) * 100.0f;
            }
            oldCoreCpu = newCoreCpu;
            oldCpu = newCpu;

            cpuTemp = getCPUTemp();
            cpuFreq = getCPUFrequency();
            getLoadAverages(loadAvg1, loadAvg5, loadAvg15);

            // Context switches & interrupts
            long long curCtxt, curIntr;
            getContextSwitchesAndInterrupts(curCtxt, curIntr);
            ctxtPerSec = (float)(curCtxt - prevCtxt);
            intrPerSec = (float)(curIntr - prevIntr);
            prevCtxt = curCtxt; prevIntr = curIntr;

            // Memory
            memDetail = getMemoryDetail();
            ramUsagePercent = memDetail.totalKB > 0 ? (float)(memDetail.totalKB - memDetail.availableKB) / (float)memDetail.totalKB : 0;
            if (ramUsagePercent * 100 > peakRam) peakRam = ramUsagePercent * 100;

            // Network
            NetStats newNet = readNetStats();
            downSpeed = newNet.rxBytes - oldNet.rxBytes;
            upSpeed = newNet.txBytes - oldNet.txBytes;
            sessionRx = newNet.rxBytes - startRx;
            sessionTx = newNet.txBytes - startTx;
            oldNet = newNet;

            // Disk I/O
            DiskStats newDisk = readDiskStats();
            diskReadSpeed = newDisk.readBytes - oldDisk.readBytes;
            diskWriteSpeed = newDisk.writeBytes - oldDisk.writeBytes;
            oldDisk = newDisk;

            // Process CPU tracking
            std::vector<ProcCPUTick> newProcTicks = readAllProcTicks();
            // total CPU jiffies for process calc
            CPUStats tmpOld = oldCpu, tmpNew = newCpu; // already saved
            long long totalCpuJiffies = totalDelta > 0 ? totalDelta : 1;
            topProcesses = getTopProcesses(oldProcTicks, newProcTicks, totalCpuJiffies, 25);
            oldProcTicks = newProcTicks;

            // Slow updates (every 5s)
            slowUpdateCounter++;
            if (slowUpdateCounter >= 5) {
                gpu = getGPUInfo();
                battery = getBatteryInfo();
                diskPartitions = getDiskPartitions();
                processCount = getProcessCount();
                threadCount = getTotalThreadCount();
                fdCount = getFileDescriptorCount();
                netConnections = getNetworkConnectionCount();
                slowUpdateCounter = 0;
            }

            // History
            cpuHistory.erase(cpuHistory.begin()); cpuHistory.push_back(cpuUsage * 100);
            ramHistory.erase(ramHistory.begin()); ramHistory.push_back(ramUsagePercent * 100);
            gpuHistory.erase(gpuHistory.begin()); gpuHistory.push_back((float)gpu.usagePercent);
            netDownHistory.erase(netDownHistory.begin()); netDownHistory.push_back((float)(downSpeed / 1024));
            netUpHistory.erase(netUpHistory.begin()); netUpHistory.push_back((float)(upSpeed / 1024));
            ioWaitHistory.erase(ioWaitHistory.begin()); ioWaitHistory.push_back(ioWaitPct);

            lastUpdateTime = currentTime;
        }

        // ==================== UI ====================
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##Main", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ===== HEADER =====
        ImGui::BeginChild("Header", ImVec2(-1, 55), false);
        {
            ImGui::SetCursorPosY(8);
            float pulse = 0.7f + 0.3f * sinf(animTime * 2.0f);
            ImVec4 titleColor = ImVec4(0.0f * pulse + 0.2f, 0.85f * pulse, 1.0f * pulse, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Text, titleColor);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Text("  CEKY MONITOR");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::TextColored(Colors::TextMuted, "v6.0 | Linux");

            float rightOffset = ImGui::GetWindowWidth() - 420;
            ImGui::SameLine(rightOffset);
            ImGui::SetCursorPosY(12);

            ImGui::TextColored(Colors::TextSecondary, "Uptime");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Cyan, "%s", getUptime().c_str());

            ImGui::SameLine(rightOffset + 140);
            ImGui::TextColored(Colors::TextSecondary, "Procs");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Purple, "%d", processCount);

            ImGui::SameLine(rightOffset + 230);
            ImGui::TextColored(Colors::TextSecondary, "Threads");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Orange, "%d", threadCount);

            ImGui::SameLine(rightOffset + 340);
            ImGui::TextColored(Colors::TextSecondary, "FDs");
            ImGui::SameLine();
            ImGui::TextColored(Colors::Teal, "%d", fdCount);
        }
        ImGui::EndChild();

        // ===== TAB BAR =====
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.10f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.30f, 1.0f));
        {
            const char* tabNames[] = {"  Overview  ", "  Processes  ", "  System Info  "};
            ImVec4 tabColors[] = { Colors::Cyan, Colors::Orange, Colors::Purple };
            for (int i = 0; i < 3; i++) {
                if (i > 0) ImGui::SameLine();
                if (currentTab == i) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(tabColors[i].x * 0.2f, tabColors[i].y * 0.2f, tabColors[i].z * 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, tabColors[i]);
                }
                if (ImGui::Button(tabNames[i], ImVec2(150, 32))) currentTab = i;
                if (currentTab == i) ImGui::PopStyleColor(2);
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::Spacing();

        // ==================== TAB: OVERVIEW ====================
        if (currentTab == 0) {
            float contentWidth = ImGui::GetContentRegionAvail().x;
            float leftWidth = contentWidth * 0.52f;
            float rightWidth = contentWidth * 0.46f;

            // LEFT PANEL
            ImGui::BeginChild("LeftPanel", ImVec2(leftWidth, -30), false);
            {
                // ===== CPU CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                float cpuCardH = 210 + (cpuCores > 4 ? 30 : 0);
                ImGui::BeginChild("CPUCard", ImVec2(-1, cpuCardH), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Orange, 0.25f);
                    CardHeader("[CPU]", "PROCESSOR", Colors::Orange);

                    ImGui::TextColored(Colors::TextPrimary, "%s", cpuModel.c_str());
                    ImGui::TextColored(Colors::TextSecondary, "%d Cores (%d Physical)  |  %.0f / %.0f MHz",
                        cpuCores, physCores, cpuFreq, cpuMaxFreq);

                    ImGui::Spacing();

                    ImGui::Columns(2, NULL, false);
                    ImGui::SetColumnWidth(0, 140);

                    char cpuVal[16];
                    snprintf(cpuVal, sizeof(cpuVal), "%.0f%%", cpuUsage * 100);
                    DrawCircularProgress(cpuUsage, 50, Colors::Orange, "USAGE", cpuVal);

                    ImGui::NextColumn();

                    // Temperature
                    ImVec4 tempColor = (cpuTemp > 80) ? Colors::Red : (cpuTemp > 60) ? Colors::Yellow : Colors::Green;
                    ImGui::TextColored(Colors::TextSecondary, "Temperature");
                    ImGui::SameLine(120);
                    ImGui::TextColored(tempColor, "%.0f C", cpuTemp);

                    ImGui::TextColored(Colors::TextSecondary, "Peak CPU");
                    ImGui::SameLine(120);
                    ImGui::TextColored(Colors::Amber, "%.0f%%", peakCpu);

                    ImGui::TextColored(Colors::TextSecondary, "IO Wait");
                    ImGui::SameLine(120);
                    float lastIOWait = ioWaitHistory.back();
                    ImGui::TextColored(lastIOWait > 5 ? Colors::Red : Colors::Green, "%.1f%%", lastIOWait);

                    ImGui::Spacing();
                    MiniGraph("##cpuGraph", cpuHistory.data(), (int)cpuHistory.size(), Colors::Orange, 45);

                    ImGui::Columns(1);

                    // Per-core bars
                    if (!perCoreUsage.empty()) {
                        ImGui::Spacing();
                        ImGui::TextColored(Colors::TextMuted, "Per-Core:");
                        int cols = perCoreUsage.size() > 8 ? 8 : (int)perCoreUsage.size();
                        float barW = (ImGui::GetContentRegionAvail().x - (cols - 1) * 4) / cols;
                        for (int i = 0; i < (int)perCoreUsage.size() && i < 16; i++) {
                            if (i > 0 && i % cols != 0) ImGui::SameLine(0, 4);
                            char coreLabel[8];
                            snprintf(coreLabel, sizeof(coreLabel), "C%d", i);
                            ImVec4 coreColor = GetUsageColor(perCoreUsage[i]);
                            DrawGradientProgressBar(perCoreUsage[i] / 100.0f, ImVec2(barW, 10), coreColor, coreColor, nullptr);
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== RAM CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("RAMCard", ImVec2(-1, 190), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Green, 0.25f);
                    CardHeader("[RAM]", "MEMORY", Colors::Green);

                    float totalGB = memDetail.totalKB / (1024.0f * 1024.0f);
                    float usedGB = (memDetail.totalKB - memDetail.availableKB) / (1024.0f * 1024.0f);
                    float cachedGB = (memDetail.cachedKB + memDetail.sreclaimableKB) / (1024.0f * 1024.0f);
                    float buffersGB = memDetail.buffersKB / (1024.0f * 1024.0f);

                    char ramStr[80];
                    snprintf(ramStr, sizeof(ramStr), "%.1f / %.1f GB  (%.0f%%)  |  Peak: %.0f%%",
                        usedGB, totalGB, ramUsagePercent * 100, peakRam);
                    ImGui::TextColored(Colors::TextPrimary, "%s", ramStr);

                    ImGui::Spacing();
                    char ramOverlay[16];
                    snprintf(ramOverlay, sizeof(ramOverlay), "%.0f%%", ramUsagePercent * 100);
                    DrawGradientProgressBar(ramUsagePercent, ImVec2(-1, 20), Colors::Green, Colors::Cyan, ramOverlay);

                    ImGui::Spacing();
                    MiniGraph("##ramGraph", ramHistory.data(), (int)ramHistory.size(), Colors::Green, 35);

                    ImGui::Spacing();
                    ImGui::Columns(3, NULL, false);
                    ImGui::TextColored(Colors::TextSecondary, "Cached");
                    ImGui::TextColored(Colors::Blue, "%.1f GB", cachedGB);
                    ImGui::NextColumn();
                    ImGui::TextColored(Colors::TextSecondary, "Buffers");
                    ImGui::TextColored(Colors::Teal, "%.2f GB", buffersGB);
                    ImGui::NextColumn();
                    ImGui::TextColored(Colors::TextSecondary, "Dirty");
                    ImGui::TextColored(Colors::Orange, "%.1f MB", memDetail.dirtyKB / 1024.0f);
                    ImGui::Columns(1);

                    // Swap
                    if (memDetail.swapTotalKB > 0) {
                        ImGui::Spacing();
                        float swapUsed = (memDetail.swapTotalKB - memDetail.swapFreeKB) / (1024.0f * 1024.0f);
                        float swapTotal = memDetail.swapTotalKB / (1024.0f * 1024.0f);
                        float swapPct = swapUsed / swapTotal;
                        ImGui::TextColored(Colors::TextSecondary, "Swap: %.1f / %.1f GB", swapUsed, swapTotal);
                        DrawGradientProgressBar(swapPct, ImVec2(-1, 10), Colors::Purple, Colors::Pink, NULL);
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== GPU CARD =====
                if (gpu.available) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                    ImGui::BeginChild("GPUCard", ImVec2(-1, 180), true, ImGuiWindowFlags_NoScrollbar);
                    {
                        DrawCardBorderGlow(Colors::Purple, 0.25f);
                        CardHeader("[GPU]", "GRAPHICS", Colors::Purple);

                        ImGui::TextColored(Colors::TextPrimary, "%s", gpu.name.c_str());

                        ImGui::Spacing();
                        ImGui::Columns(2, NULL, false);

                        if (gpu.temp > 0) {
                            ImVec4 tc = (gpu.temp > 80) ? Colors::Red : (gpu.temp > 60) ? Colors::Yellow : Colors::Green;
                            ImGui::TextColored(Colors::TextSecondary, "Temp");
                            ImGui::SameLine(70);
                            ImGui::TextColored(tc, "%d C", gpu.temp);
                        }
                        if (gpu.usagePercent >= 0) {
                            ImGui::TextColored(Colors::TextSecondary, "Usage");
                            ImGui::SameLine(70);
                            ImGui::TextColored(Colors::Purple, "%d%%", gpu.usagePercent);
                        }
                        if (gpu.fanSpeed > 0) {
                            ImGui::TextColored(Colors::TextSecondary, "Fan");
                            ImGui::SameLine(70);
                            ImGui::TextColored(Colors::Cyan, "%d%%", gpu.fanSpeed);
                        }

                        ImGui::NextColumn();

                        if (gpu.memTotalMB > 0) {
                            ImGui::TextColored(Colors::TextSecondary, "VRAM");
                            ImGui::SameLine(70);
                            ImGui::TextColored(Colors::Cyan, "%d/%d MB", gpu.memUsedMB, gpu.memTotalMB);
                        }
                        if (gpu.powerDraw > 0) {
                            ImGui::TextColored(Colors::TextSecondary, "Power");
                            ImGui::SameLine(70);
                            ImGui::TextColored(Colors::Amber, "%d W", gpu.powerDraw);
                        }

                        ImGui::Columns(1);

                        ImGui::Spacing();
                        if (gpu.usagePercent >= 0) {
                            char gpuOvl[16];
                            snprintf(gpuOvl, sizeof(gpuOvl), "%d%%", gpu.usagePercent);
                            DrawGradientProgressBar(gpu.usagePercent / 100.0f, ImVec2(-1, 16), Colors::Purple, Colors::Pink, gpuOvl);
                        }
                        if (gpu.memTotalMB > 0) {
                            ImGui::Spacing();
                            DrawGradientProgressBar(gpu.memUsedMB / (float)gpu.memTotalMB, ImVec2(-1, 10), Colors::Blue, Colors::Cyan, NULL);
                        }

                        ImGui::Spacing();
                        MiniGraph("##gpuGraph", gpuHistory.data(), (int)gpuHistory.size(), Colors::Purple, 30);
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 12);

            // RIGHT PANEL
            ImGui::BeginChild("RightPanel", ImVec2(rightWidth, -30), false);
            {
                // ===== NETWORK CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("NetCard", ImVec2(-1, 220), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Cyan, 0.25f);
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

                    ImGui::TextColored(Colors::TextSecondary, "Session Total:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Cyan, "RX: %s", formatBytes(sessionRx).c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Pink, "TX: %s", formatBytes(sessionTx).c_str());

                    ImGui::TextColored(Colors::TextSecondary, "TCP Connections:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Teal, "%d", netConnections);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== DISK I/O CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("DiskIOCard", ImVec2(-1, 80), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Amber, 0.2f);
                    CardHeader("[I/O]", "DISK I/O", Colors::Amber);

                    ImGui::Columns(2, NULL, false);
                    ImGui::TextColored(Colors::Green, "Read: %s", formatSpeed(diskReadSpeed).c_str());
                    ImGui::TextColored(Colors::TextMuted, "Total: %s", formatBytes(oldDisk.readBytes - startDiskRead).c_str());
                    ImGui::NextColumn();
                    ImGui::TextColored(Colors::Orange, "Write: %s", formatSpeed(diskWriteSpeed).c_str());
                    ImGui::TextColored(Colors::TextMuted, "Total: %s", formatBytes(oldDisk.writeBytes - startDiskWrite).c_str());
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== STORAGE CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                float diskH = 55.0f + diskPartitions.size() * 48.0f;
                if (diskH > 220) diskH = 220;
                ImGui::BeginChild("DiskCard", ImVec2(-1, diskH), true);
                {
                    DrawCardBorderGlow(Colors::Yellow, 0.2f);
                    CardHeader("[HDD]", "STORAGE", Colors::Yellow);

                    for (const auto& d : diskPartitions) {
                        ImGui::TextColored(Colors::TextPrimary, "%s", d.name.c_str());
                        ImGui::SameLine(120);
                        ImGui::TextColored(Colors::TextMuted, "[%s]", d.fsType.c_str());
                        ImGui::SameLine(180);
                        ImGui::TextColored(Colors::TextSecondary, "%.1f / %.1f GB", d.usedGB, d.totalGB);

                        ImVec4 dc = (d.usagePercent > 0.9f) ? Colors::Red :
                                    (d.usagePercent > 0.7f) ? Colors::Yellow : Colors::Green;
                        char dovl[8];
                        snprintf(dovl, sizeof(dovl), "%.0f%%", d.usagePercent * 100);
                        DrawGradientProgressBar(d.usagePercent, ImVec2(-1, 14), dc, dc, dovl);
                        ImGui::Spacing();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== LOAD & KERNEL CARD =====
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::BeginChild("LoadCard", ImVec2(-1, 100), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Teal, 0.2f);
                    CardHeader("[SYS]", "LOAD & KERNEL", Colors::Teal);

                    ImGui::TextColored(Colors::TextSecondary, "Load Avg:");
                    ImGui::SameLine();
                    ImGui::TextColored(GetUsageColor(loadAvg1 / cpuCores * 100), "%.2f", loadAvg1);
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::TextSecondary, "/");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Yellow, "%.2f", loadAvg5);
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::TextSecondary, "/");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Blue, "%.2f", loadAvg15);

                    ImGui::TextColored(Colors::TextSecondary, "Ctx Switch/s:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Cyan, "%.0f", ctxtPerSec);
                    ImGui::SameLine(0, 20);
                    ImGui::TextColored(Colors::TextSecondary, "IRQ/s:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Orange, "%.0f", intrPerSec);

                    ImGui::TextColored(Colors::TextSecondary, "IO Wait:");
                    ImGui::SameLine();
                    MiniGraph("##iowait", ioWaitHistory.data(), (int)ioWaitHistory.size(), Colors::Red, 20);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Spacing();

                // ===== BATTERY CARD =====
                if (battery.available) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                    ImGui::BeginChild("BatteryCard", ImVec2(-1, 100), true, ImGuiWindowFlags_NoScrollbar);
                    {
                        DrawCardBorderGlow(Colors::Green, 0.2f);
                        CardHeader("[BAT]", "BATTERY", Colors::Green);

                        ImVec4 bc = (battery.percent < 20) ? Colors::Red :
                                    (battery.percent < 50) ? Colors::Yellow : Colors::Green;

                        const char* status = battery.charging ? "Charging" : "On Battery";
                        ImGui::TextColored(bc, "%d%%", battery.percent);
                        ImGui::SameLine();
                        ImGui::TextColored(Colors::TextSecondary, "%s", status);

                        if (battery.powerWatts > 0) {
                            ImGui::SameLine(0, 20);
                            ImGui::TextColored(Colors::Orange, "%.1f W", battery.powerWatts);
                        }
                        if (battery.estimatedMinutes > 0) {
                            ImGui::SameLine(0, 20);
                            int hrs = battery.estimatedMinutes / 60;
                            int mins = battery.estimatedMinutes % 60;
                            ImGui::TextColored(Colors::Cyan, "%dh %dm left", hrs, mins);
                        }

                        ImGui::Spacing();
                        char batOvl[16];
                        snprintf(batOvl, sizeof(batOvl), "%d%%", battery.percent);
                        DrawGradientProgressBar(battery.percent / 100.0f, ImVec2(-1, 18), bc, Colors::Green, batOvl);
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();
        }
        // ==================== TAB: PROCESSES ====================
        else if (currentTab == 1) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::BeginChild("ProcessPanel", ImVec2(-1, -30), true);
            {
                DrawCardBorderGlow(Colors::Orange, 0.25f);
                CardHeader("[TOP]", "TOP PROCESSES", Colors::Orange);

                ImGui::TextColored(Colors::TextSecondary, "Total: %d processes  |  %d threads  |  %d file descriptors",
                    processCount, threadCount, fdCount);
                ImGui::Spacing();

                if (ImGui::BeginTable("ProcessTable", 6,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                    ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp)) {

                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 70);
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 50);
                    ImGui::TableSetupColumn("CPU %", ImGuiTableColumnFlags_WidthFixed, 80);
                    ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 90);
                    ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_WidthFixed, 70);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    for (const auto& p : topProcesses) {
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(Colors::TextMuted, "%d", p.pid);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextColored(Colors::TextPrimary, "%s", p.name.c_str());

                        ImGui::TableSetColumnIndex(2);
                        ImVec4 stateColor = (p.state == 'R') ? Colors::Green : (p.state == 'S') ? Colors::Blue : Colors::Yellow;
                        char stateStr[4] = {p.state, 0};
                        ImGui::TextColored(stateColor, "%s", stateStr);

                        ImGui::TableSetColumnIndex(3);
                        ImVec4 cpuColor = GetUsageColor(p.cpuPercent);
                        ImGui::TextColored(cpuColor, "%.1f%%", p.cpuPercent);

                        ImGui::TableSetColumnIndex(4);
                        if (p.memMB > 1024)
                            ImGui::TextColored(Colors::Cyan, "%.1f GB", p.memMB / 1024.0f);
                        else
                            ImGui::TextColored(Colors::Cyan, "%.1f MB", p.memMB);

                        ImGui::TableSetColumnIndex(5);
                        ImGui::TextColored(Colors::TextSecondary, "%d", p.threadCount);
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        // ==================== TAB: SYSTEM INFO ====================
        else if (currentTab == 2) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::BeginChild("SysInfoPanel", ImVec2(-1, -30), true);
            {
                DrawCardBorderGlow(Colors::Purple, 0.25f);
                CardHeader("[INF]", "SYSTEM INFORMATION", Colors::Purple);
                ImGui::Spacing();

                // OS Section
                ImGui::TextColored(Colors::Cyan, "--- Operating System ---");
                ImGui::Spacing();
                DrawStatRow("Distribution", sysInfo.distroName.c_str(), Colors::Green);
                DrawStatRow("Kernel", sysInfo.kernelVersion.c_str(), Colors::Cyan);
                DrawStatRow("Architecture", sysInfo.architecture.c_str(), Colors::Orange);
                DrawStatRow("Hostname", sysInfo.hostname.c_str(), Colors::Purple);
                DrawStatRow("Username", sysInfo.username.c_str(), Colors::Pink);
                DrawStatRow("Uptime", getUptime().c_str(), Colors::Yellow);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // CPU Section
                ImGui::TextColored(Colors::Orange, "--- Processor ---");
                ImGui::Spacing();
                DrawStatRow("Model", cpuModel.c_str(), Colors::TextPrimary);

                char coreStr[64];
                snprintf(coreStr, sizeof(coreStr), "%d Logical / %d Physical", sysInfo.logicalCores, sysInfo.physicalCores);
                DrawStatRow("Cores", coreStr, Colors::Cyan);

                char freqStr[64];
                snprintf(freqStr, sizeof(freqStr), "%.0f MHz (Max: %.0f MHz)", cpuFreq, cpuMaxFreq);
                DrawStatRow("Frequency", freqStr, Colors::Orange);

                char tempStr[32];
                snprintf(tempStr, sizeof(tempStr), "%.0f C", cpuTemp);
                DrawStatRow("Temperature", tempStr, cpuTemp > 80 ? Colors::Red : Colors::Green);

                char loadStr[64];
                snprintf(loadStr, sizeof(loadStr), "%.2f / %.2f / %.2f", loadAvg1, loadAvg5, loadAvg15);
                DrawStatRow("Load Average", loadStr, Colors::Yellow);

                char ctxtStr[64];
                snprintf(ctxtStr, sizeof(ctxtStr), "%.0f / sec", ctxtPerSec);
                DrawStatRow("Ctx Switches", ctxtStr, Colors::Teal);

                char intrStr[64];
                snprintf(intrStr, sizeof(intrStr), "%.0f / sec", intrPerSec);
                DrawStatRow("Interrupts", intrStr, Colors::Blue);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Memory Section
                ImGui::TextColored(Colors::Green, "--- Memory ---");
                ImGui::Spacing();

                char totalStr[32];
                snprintf(totalStr, sizeof(totalStr), "%.1f GB", memDetail.totalKB / (1024.0f * 1024.0f));
                DrawStatRow("Total RAM", totalStr, Colors::TextPrimary);

                char usedStr[32];
                snprintf(usedStr, sizeof(usedStr), "%.1f GB (%.0f%%)",
                    (memDetail.totalKB - memDetail.availableKB) / (1024.0f * 1024.0f), ramUsagePercent * 100);
                DrawStatRow("Used", usedStr, Colors::Orange);

                char cachedStr[32];
                snprintf(cachedStr, sizeof(cachedStr), "%.1f GB", (memDetail.cachedKB + memDetail.sreclaimableKB) / (1024.0f * 1024.0f));
                DrawStatRow("Cached", cachedStr, Colors::Blue);

                char bufStr[32];
                snprintf(bufStr, sizeof(bufStr), "%.0f MB", memDetail.buffersKB / 1024.0f);
                DrawStatRow("Buffers", bufStr, Colors::Teal);

                char activeStr[32];
                snprintf(activeStr, sizeof(activeStr), "%.1f GB", memDetail.activeKB / (1024.0f * 1024.0f));
                DrawStatRow("Active", activeStr, Colors::Cyan);

                char inactiveStr[32];
                snprintf(inactiveStr, sizeof(inactiveStr), "%.1f GB", memDetail.inactiveKB / (1024.0f * 1024.0f));
                DrawStatRow("Inactive", inactiveStr, Colors::TextSecondary);

                char dirtyStr[32];
                snprintf(dirtyStr, sizeof(dirtyStr), "%.1f MB", memDetail.dirtyKB / 1024.0f);
                DrawStatRow("Dirty", dirtyStr, Colors::Red);

                if (memDetail.swapTotalKB > 0) {
                    char swapStr[64];
                    snprintf(swapStr, sizeof(swapStr), "%.1f / %.1f GB",
                        (memDetail.swapTotalKB - memDetail.swapFreeKB) / (1024.0f * 1024.0f),
                        memDetail.swapTotalKB / (1024.0f * 1024.0f));
                    DrawStatRow("Swap", swapStr, Colors::Purple);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // GPU Section
                if (gpu.available) {
                    ImGui::TextColored(Colors::Purple, "--- Graphics ---");
                    ImGui::Spacing();
                    DrawStatRow("GPU", gpu.name.c_str(), Colors::TextPrimary);
                    if (gpu.temp > 0) {
                        char gtStr[16]; snprintf(gtStr, sizeof(gtStr), "%d C", gpu.temp);
                        DrawStatRow("GPU Temp", gtStr, gpu.temp > 80 ? Colors::Red : Colors::Green);
                    }
                    if (gpu.memTotalMB > 0) {
                        char vmStr[32]; snprintf(vmStr, sizeof(vmStr), "%d / %d MB", gpu.memUsedMB, gpu.memTotalMB);
                        DrawStatRow("VRAM", vmStr, Colors::Cyan);
                    }
                    if (gpu.fanSpeed > 0) {
                        char fnStr[16]; snprintf(fnStr, sizeof(fnStr), "%d%%", gpu.fanSpeed);
                        DrawStatRow("Fan Speed", fnStr, Colors::Teal);
                    }
                    if (gpu.powerDraw > 0) {
                        char pwStr[16]; snprintf(pwStr, sizeof(pwStr), "%d W", gpu.powerDraw);
                        DrawStatRow("Power Draw", pwStr, Colors::Amber);
                    }
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }

                // Storage Section
                ImGui::TextColored(Colors::Yellow, "--- Storage ---");
                ImGui::Spacing();
                for (const auto& d : diskPartitions) {
                    char dStr[80];
                    snprintf(dStr, sizeof(dStr), "%.1f / %.1f GB (%.0f%%) [%s]",
                        d.usedGB, d.totalGB, d.usagePercent * 100, d.fsType.c_str());
                    DrawStatRow(d.name.c_str(), dStr, d.usagePercent > 0.9f ? Colors::Red : Colors::Green);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Network Section
                ImGui::TextColored(Colors::Cyan, "--- Network ---");
                ImGui::Spacing();
                DrawStatRow("Session RX", formatBytes(sessionRx).c_str(), Colors::Cyan);
                DrawStatRow("Session TX", formatBytes(sessionTx).c_str(), Colors::Pink);
                char connStr[16]; snprintf(connStr, sizeof(connStr), "%d", netConnections);
                DrawStatRow("TCP Connections", connStr, Colors::Teal);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        // ===== FOOTER STATUS BAR =====
        {
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 26);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::HeaderBg);
            ImGui::BeginChild("Footer", ImVec2(-1, 26), false);
            ImGui::SetCursorPosX(8);
            ImGui::SetCursorPosY(4);
            ImGui::TextColored(Colors::TextMuted, "Ceky Monitor v6.0");
            ImGui::SameLine(0, 20);
            ImGui::TextColored(Colors::TextMuted, "|");
            ImGui::SameLine(0, 20);
            ImGui::TextColored(Colors::TextSecondary, "CPU: %.0f%%", cpuUsage * 100);
            ImGui::SameLine(0, 15);
            ImGui::TextColored(Colors::TextSecondary, "RAM: %.0f%%", ramUsagePercent * 100);
            ImGui::SameLine(0, 15);
            ImGui::TextColored(Colors::TextSecondary, "Load: %.2f", loadAvg1);
            if (gpu.available) {
                ImGui::SameLine(0, 15);
                ImGui::TextColored(Colors::TextSecondary, "GPU: %d%%", gpu.usagePercent);
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 160);
            ImGui::TextColored(Colors::TextMuted, "Update: 1s | History: %ds", HISTORY_SIZE);
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        ImGui::End();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.04f, 0.04f, 0.07f, 1.0f);
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

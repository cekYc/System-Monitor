// CekyMonitor v6.0 - Premium Windows System Monitor
// Features: CPU, RAM, GPU, Disk, Network, Battery, Frequency, Processes, Uptime
//           Top Processes, System Info, Thread/Handle Count, Per-Core CPU, Disk I/O
//           Network Total Traffic, OS Info, Hostname, Username
// UI: Modern Glassmorphism, Neon Accents, Smooth Animations, Card Layout, Tab Navigation

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <GLFW/glfw3.h>

// Windows API
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <tlhelp32.h>
#include <winioctl.h>

// ==================== DATA STRUCTURES ====================
struct NetStats { long long rxBytes, txBytes; };
struct DiskInfo { std::string name; float totalGB, usedGB, freeGB, usagePercent; std::string fileSystem; };
struct GPUInfo { std::string name; int temp; int usagePercent; int memUsedMB, memTotalMB; int fanSpeed; int powerDraw; bool available; };
struct BatteryInfo { int percent; bool charging; bool available; float powerWatts; int estimatedMinutes; };
struct ProcessInfo { std::string name; DWORD pid; float cpuPercent; float memMB; int threadCount; };
struct SystemInfo { 
    std::string osName, osVersion, hostname, username, architecture;
    int logicalCores, physicalCores; 
    float totalRamGB;
};
struct DiskIOInfo { double readBytesPerSec, writeBytesPerSec; long long totalRead, totalWrite; };

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
    
    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]          = Colors::Background;
    c[ImGuiCol_ChildBg]           = Colors::CardBg;
    c[ImGuiCol_PopupBg]           = ImVec4(0.08f, 0.08f, 0.11f, 0.98f);
    c[ImGuiCol_Border]            = Colors::CardBorder;
    c[ImGuiCol_BorderShadow]      = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_Text]              = Colors::TextPrimary;
    c[ImGuiCol_TextDisabled]      = Colors::TextMuted;
    c[ImGuiCol_FrameBg]           = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.17f, 0.17f, 0.21f, 1.0f);
    c[ImGuiCol_FrameBgActive]     = ImVec4(0.22f, 0.22f, 0.27f, 1.0f);
    c[ImGuiCol_TitleBg]           = Colors::HeaderBg;
    c[ImGuiCol_TitleBgActive]     = Colors::HeaderBg;
    c[ImGuiCol_TitleBgCollapsed]  = Colors::HeaderBg;
    c[ImGuiCol_ScrollbarBg]       = ImVec4(0.06f, 0.06f, 0.08f, 0.4f);
    c[ImGuiCol_ScrollbarGrab]     = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
    c[ImGuiCol_Button]            = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.22f, 0.22f, 0.28f, 1.0f);
    c[ImGuiCol_ButtonActive]      = ImVec4(0.28f, 0.28f, 0.35f, 1.0f);
    c[ImGuiCol_Header]            = ImVec4(0.15f, 0.15f, 0.20f, 0.6f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.20f, 0.20f, 0.26f, 0.8f);
    c[ImGuiCol_HeaderActive]      = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
    c[ImGuiCol_Separator]         = ImVec4(0.18f, 0.18f, 0.22f, 0.5f);
    c[ImGuiCol_SeparatorHovered]  = Colors::Cyan;
    c[ImGuiCol_SeparatorActive]   = Colors::Cyan;
    c[ImGuiCol_Tab]               = ImVec4(0.12f, 0.12f, 0.16f, 1.0f);
    c[ImGuiCol_TabHovered]        = ImVec4(0.20f, 0.20f, 0.26f, 1.0f);
    c[ImGuiCol_TabActive]         = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
    c[ImGuiCol_PlotLines]         = Colors::Cyan;
    c[ImGuiCol_PlotLinesHovered]  = Colors::Pink;
    c[ImGuiCol_PlotHistogram]     = Colors::Purple;
    c[ImGuiCol_PlotHistogramHovered] = Colors::Pink;
    c[ImGuiCol_TableHeaderBg]     = ImVec4(0.12f, 0.12f, 0.16f, 1.0f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
    c[ImGuiCol_TableBorderLight]  = ImVec4(0.14f, 0.14f, 0.18f, 1.0f);
    c[ImGuiCol_TableRowBg]        = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_TableRowBgAlt]     = ImVec4(0.10f, 0.10f, 0.13f, 0.4f);
}

// ==================== CUSTOM DRAWING FUNCTIONS ====================
void DrawGradientProgressBar(float fraction, ImVec2 size, ImVec4 colorLeft, ImVec4 colorRight, const char* overlay = nullptr) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    
    ImVec2 pos = window->DC.CursorPos;
    if (size.x < 0) size.x = ImGui::GetContentRegionAvail().x;
    ImVec2 endPos = ImVec2(pos.x + size.x, pos.y + size.y);
    
    // Background track with subtle inner shadow
    window->DrawList->AddRectFilled(pos, endPos, IM_COL32(20, 20, 25, 255), size.y * 0.5f);
    window->DrawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 1), IM_COL32(10, 10, 12, 180), size.y * 0.5f);
    
    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        ImVec2 fillEnd = ImVec2(pos.x + size.x * fraction, endPos.y);
        ImU32 colL = ImGui::ColorConvertFloat4ToU32(colorLeft);
        ImU32 colR = ImGui::ColorConvertFloat4ToU32(colorRight);
        window->DrawList->AddRectFilledMultiColor(pos, fillEnd, colL, colR, colR, colL);
        // Subtle glow highlight on top
        ImU32 highlightCol = IM_COL32(255, 255, 255, 25);
        window->DrawList->AddRectFilled(pos, ImVec2(fillEnd.x, pos.y + size.y * 0.4f), highlightCol, size.y * 0.5f);
    }
    
    if (overlay) {
        ImVec2 textSize = ImGui::CalcTextSize(overlay);
        ImVec2 textPos = ImVec2(pos.x + (size.x - textSize.x) * 0.5f, pos.y + (size.y - textSize.y) * 0.5f);
        // Text shadow
        window->DrawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 180), overlay);
        window->DrawList->AddText(textPos, IM_COL32(255, 255, 255, 255), overlay);
    }
    
    ImGui::ItemSize(size);
}

void DrawCircularProgress(float fraction, float radius, ImVec4 color, const char* label, const char* value) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    float thickness = 5.0f;
    
    // Background ring
    window->DrawList->AddCircle(center, radius, IM_COL32(30, 30, 40, 255), 48, thickness);
    
    if (fraction > 0.0f) {
        if (fraction > 1.0f) fraction = 1.0f;
        float startAngle = -3.14159f / 2.0f;
        float endAngle = startAngle + fraction * 2.0f * 3.14159f;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
        
        int segments = (int)(fraction * 48);
        if (segments < 3) segments = 3;
        
        for (int i = 0; i < segments; i++) {
            float a1 = startAngle + (endAngle - startAngle) * (float)i / segments;
            float a2 = startAngle + (endAngle - startAngle) * (float)(i + 1) / segments;
            ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
            ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
            window->DrawList->AddLine(p1, p2, col, thickness + 1.0f);
        }
        // Glow dot at end of arc
        float ea = endAngle;
        ImVec2 dotPos = ImVec2(center.x + cosf(ea) * radius, center.y + sinf(ea) * radius);
        window->DrawList->AddCircleFilled(dotPos, thickness, col);
    }
    
    // Center value
    ImVec2 valueSize = ImGui::CalcTextSize(value);
    window->DrawList->AddText(ImVec2(center.x - valueSize.x * 0.5f, center.y - 10), IM_COL32(255, 255, 255, 255), value);
    
    // Label below
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    window->DrawList->AddText(ImVec2(center.x - labelSize.x * 0.5f, center.y + 8), IM_COL32(140, 140, 155, 255), label);
    
    ImGui::ItemSize(ImVec2(radius * 2, radius * 2));
}

void DrawCardBorderGlow(ImVec4 glowColor, float alpha = 0.15f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 p0 = window->Pos;
    ImVec2 p1 = ImVec2(p0.x + window->Size.x, p0.y + window->Size.y);
    ImU32 col = IM_COL32((int)(glowColor.x * 255), (int)(glowColor.y * 255), (int)(glowColor.z * 255), (int)(alpha * 255));
    window->DrawList->AddRect(p0, p1, col, 12.0f, 0, 1.5f);
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
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.07f, 0.07f, 0.09f, 0.8f));
    ImGui::PlotLines(id, data, count, 0, NULL, 0, 100, ImVec2(-1, height));
    ImGui::PopStyleColor(2);
}

void DrawStatRow(const char* label, const char* value, ImVec4 valueColor, float labelWidth = 100.0f) {
    ImGui::TextColored(Colors::TextSecondary, "%s", label);
    ImGui::SameLine(labelWidth);
    ImGui::TextColored(valueColor, "%s", value);
}

ImVec4 GetUsageColor(float fraction) {
    if (fraction > 0.9f) return Colors::Red;
    if (fraction > 0.75f) return Colors::Orange;
    if (fraction > 0.5f) return Colors::Yellow;
    return Colors::Green;
}

// ==================== CPU FUNCTIONS ====================
static ULARGE_INTEGER lastIdleTime, lastKernelTime, lastUserTime;
static bool cpuInitialized = false;

void initCPU() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        lastIdleTime.LowPart = idleTime.dwLowDateTime;
        lastIdleTime.HighPart = idleTime.dwHighDateTime;
        lastKernelTime.LowPart = kernelTime.dwLowDateTime;
        lastKernelTime.HighPart = kernelTime.dwHighDateTime;
        lastUserTime.LowPart = userTime.dwLowDateTime;
        lastUserTime.HighPart = userTime.dwHighDateTime;
        cpuInitialized = true;
    }
}

float getCPUUsage() {
    if (!cpuInitialized) { initCPU(); return 0.0f; }
    
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0f;
    
    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;     idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;  kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;      user.HighPart = userTime.dwHighDateTime;
    
    ULONGLONG idleDiff = idle.QuadPart - lastIdleTime.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - lastKernelTime.QuadPart;
    ULONGLONG userDiff = user.QuadPart - lastUserTime.QuadPart;
    ULONGLONG totalTime = kernelDiff + userDiff;
    
    lastIdleTime = idle; lastKernelTime = kernel; lastUserTime = user;
    
    if (totalTime == 0) return 0.0f;
    return (float)(totalTime - idleDiff) / (float)totalTime;
}

std::string getCPUModel() {
    int cpuInfo[4] = {0};
    char brand[49] = {0};
    __cpuid(cpuInfo, 0x80000002); memcpy(brand, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000003); memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000004); memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
    
    std::string model = brand;
    size_t start = model.find_first_not_of(' ');
    if (start != std::string::npos) model = model.substr(start);
    // Trim trailing spaces
    size_t end = model.find_last_not_of(' ');
    if (end != std::string::npos) model = model.substr(0, end + 1);
    if (model.length() > 45) model = model.substr(0, 42) + "...";
    return model;
}

int getCPUPhysicalCoreCount() {
    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    if (!GetLogicalProcessorInformation(buffer.data(), &length)) return 0;
    int cores = 0;
    for (auto& info : buffer) {
        if (info.Relationship == RelationProcessorCore) cores++;
    }
    return cores;
}

int getCPULogicalCoreCount() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

float getCPUFrequency() {
    HKEY hKey;
    DWORD freq = 0, size = sizeof(freq);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&freq, &size);
        RegCloseKey(hKey);
    }
    return (float)freq;
}

// ==================== RAM FUNCTIONS ====================
float getRamUsage(float& total, float& available, float& swapTotal, float& swapUsed, float& committed) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    total = (float)(memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    available = (float)(memInfo.ullAvailPhys / (1024.0 * 1024.0 * 1024.0));
    
    swapTotal = (float)((memInfo.ullTotalPageFile - memInfo.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0));
    float swapAvail = (float)((memInfo.ullAvailPageFile - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0));
    swapUsed = swapTotal - swapAvail;
    if (swapUsed < 0) swapUsed = 0;
    if (swapTotal < 0) swapTotal = 0;
    
    // Committed memory
    PERFORMANCE_INFORMATION perfInfo;
    perfInfo.cb = sizeof(perfInfo);
    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        committed = (float)((perfInfo.CommitTotal * perfInfo.PageSize) / (1024.0 * 1024.0 * 1024.0));
    } else {
        committed = 0;
    }
    
    return (float)memInfo.dwMemoryLoad / 100.0f;
}

// ==================== NETWORK FUNCTIONS ====================
NetStats readNetStats() {
    NetStats stats = {0, 0};
    DWORD dwSize = 0;
    GetIfTable(NULL, &dwSize, FALSE);
    if (dwSize > 0) {
        MIB_IFTABLE* pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable && GetIfTable(pIfTable, &dwSize, FALSE) == NO_ERROR) {
            for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
                MIB_IFROW& row = pIfTable->table[i];
                if (row.dwType != IF_TYPE_SOFTWARE_LOOPBACK && 
                    row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL) {
                    stats.rxBytes += row.dwInOctets;
                    stats.txBytes += row.dwOutOctets;
                }
            }
            free(pIfTable);
        }
    }
    return stats;
}

int getActiveNetworkAdapters() {
    int count = 0;
    DWORD dwSize = 0;
    GetIfTable(NULL, &dwSize, FALSE);
    if (dwSize > 0) {
        MIB_IFTABLE* pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable && GetIfTable(pIfTable, &dwSize, FALSE) == NO_ERROR) {
            for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
                MIB_IFROW& row = pIfTable->table[i];
                if (row.dwType != IF_TYPE_SOFTWARE_LOOPBACK && 
                    row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL) {
                    count++;
                }
            }
            free(pIfTable);
        }
    }
    return count;
}

// ==================== DISK FUNCTIONS ====================
std::vector<DiskInfo> getDiskPartitions() {
    std::vector<DiskInfo> disks;
    DWORD drives = GetLogicalDrives();
    char driveLetter[4] = "A:\\";
    
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            driveLetter[0] = 'A' + i;
            UINT driveType = GetDriveTypeA(driveLetter);
            if (driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE) {
                ULARGE_INTEGER freeBytesAvail, totalBytes, freeBytes;
                if (GetDiskFreeSpaceExA(driveLetter, &freeBytesAvail, &totalBytes, &freeBytes)) {
                    DiskInfo info;
                    info.name = driveLetter;
                    info.name.pop_back(); // Remove trailing backslash
                    info.totalGB = (float)(totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0));
                    info.usedGB = (float)((totalBytes.QuadPart - freeBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0));
                    info.freeGB = (float)(freeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0));
                    info.usagePercent = (info.totalGB > 0) ? (info.usedGB / info.totalGB) : 0;
                    
                    // Get file system type
                    char fsName[32] = {0};
                    char volName[256] = {0};
                    if (GetVolumeInformationA(driveLetter, volName, sizeof(volName), 
                        NULL, NULL, NULL, fsName, sizeof(fsName))) {
                        info.fileSystem = fsName;
                    }
                    
                    if (info.totalGB > 0.1f) disks.push_back(info);
                }
            }
        }
    }
    return disks;
}

DiskIOInfo getDiskIO() {
    static long long lastRead = 0, lastWrite = 0;
    static bool first = true;
    DiskIOInfo io = {0, 0, 0, 0};
    
    // Read from all physical drives
    long long totalRead = 0, totalWrite = 0;
    for (int driveIdx = 0; driveIdx < 16; driveIdx++) {
        char path[64];
        snprintf(path, sizeof(path), "\\\\.\\PhysicalDrive%d", driveIdx);
        HANDLE hDrive = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive != INVALID_HANDLE_VALUE) {
            DISK_PERFORMANCE perf = {};
            DWORD bytesReturned;
            if (DeviceIoControl(hDrive, IOCTL_DISK_PERFORMANCE, NULL, 0, &perf, sizeof(perf), &bytesReturned, NULL)) {
                totalRead += perf.BytesRead.QuadPart;
                totalWrite += perf.BytesWritten.QuadPart;
            }
            CloseHandle(hDrive);
        }
    }
    
    io.totalRead = totalRead;
    io.totalWrite = totalWrite;
    
    if (!first) {
        io.readBytesPerSec = (double)(totalRead - lastRead);
        io.writeBytesPerSec = (double)(totalWrite - lastWrite);
        if (io.readBytesPerSec < 0) io.readBytesPerSec = 0;
        if (io.writeBytesPerSec < 0) io.writeBytesPerSec = 0;
    }
    first = false;
    lastRead = totalRead;
    lastWrite = totalWrite;
    
    return io;
}

// ==================== GPU FUNCTIONS ====================
GPUInfo getGPUInfo() {
    GPUInfo gpu = {"", 0, 0, 0, 0, 0, 0, false};
    
    FILE* pipe = _popen("nvidia-smi --query-gpu=name,temperature.gpu,utilization.gpu,memory.used,memory.total,fan.speed,power.draw --format=csv,noheader,nounits 2>nul", "r");
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
            size_t start = gpu.name.find_first_not_of(' ');
            size_t end2 = gpu.name.find_last_not_of(" \r\n");
            if (start != std::string::npos && end2 != std::string::npos)
                gpu.name = gpu.name.substr(start, end2 - start + 1);
            if (gpu.name.length() > 30) gpu.name = gpu.name.substr(0, 27) + "...";
            
            try {
                gpu.temp = std::stoi(temp);
                gpu.usagePercent = std::stoi(usage);
                gpu.memUsedMB = std::stoi(memUsed);
                gpu.memTotalMB = std::stoi(memTotal);
                try { gpu.fanSpeed = std::stoi(fan); } catch (...) { gpu.fanSpeed = -1; }
                try { gpu.powerDraw = (int)std::stof(power); } catch (...) { gpu.powerDraw = -1; }
                gpu.available = true;
            } catch (...) {}
        }
        _pclose(pipe);
    }
    return gpu;
}

// ==================== BATTERY FUNCTIONS ====================
BatteryInfo getBatteryInfo() {
    BatteryInfo bat = {0, false, false, 0, -1};
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus)) {
        if (powerStatus.BatteryFlag != 128 && powerStatus.BatteryFlag != 255) {
            bat.available = true;
            bat.percent = powerStatus.BatteryLifePercent;
            if (bat.percent > 100) bat.percent = 100;
            bat.charging = (powerStatus.ACLineStatus == 1);
            if (powerStatus.BatteryLifeTime != (DWORD)-1) {
                bat.estimatedMinutes = (int)(powerStatus.BatteryLifeTime / 60);
            }
        }
    }
    return bat;
}

// ==================== SYSTEM INFO ====================
std::string getUptime() {
    ULONGLONG uptimeMs = GetTickCount64();
    int totalSecs = (int)(uptimeMs / 1000);
    int days = totalSecs / 86400;
    int hours = (totalSecs % 86400) / 3600;
    int mins = (totalSecs % 3600) / 60;
    int secs = totalSecs % 60;
    
    std::stringstream ss;
    if (days > 0) ss << days << "d ";
    ss << std::setfill('0') << std::setw(2) << hours << ":" 
       << std::setfill('0') << std::setw(2) << mins << ":" 
       << std::setfill('0') << std::setw(2) << secs;
    return ss.str();
}

int getProcessCount() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    int count = 0;
    if (Process32First(hSnap, &pe)) { do { count++; } while (Process32Next(hSnap, &pe)); }
    CloseHandle(hSnap);
    return count;
}

int getThreadCount() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;
    THREADENTRY32 te; te.dwSize = sizeof(te);
    int count = 0;
    if (Thread32First(hSnap, &te)) { do { count++; } while (Thread32Next(hSnap, &te)); }
    CloseHandle(hSnap);
    return count;
}

int getHandleCount() {
    PERFORMANCE_INFORMATION perfInfo;
    perfInfo.cb = sizeof(perfInfo);
    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        return (int)perfInfo.HandleCount;
    }
    return 0;
}

std::vector<ProcessInfo> getTopProcesses(int maxCount = 10) {
    std::vector<ProcessInfo> procs;
    
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return procs;
    
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    if (Process32First(hSnap, &pe)) {
        do {
            if (pe.th32ProcessID == 0) continue;
            
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    ProcessInfo info;
                    info.name = pe.szExeFile;
                    info.pid = pe.th32ProcessID;
                    info.memMB = (float)(pmc.WorkingSetSize / (1024.0 * 1024.0));
                    info.threadCount = pe.cntThreads;
                    info.cpuPercent = 0; // Would need delta measurement for accurate CPU
                    procs.push_back(info);
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    
    // Sort by memory usage descending
    std::sort(procs.begin(), procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.memMB > b.memMB;
    });
    
    if ((int)procs.size() > maxCount) procs.resize(maxCount);
    return procs;
}

SystemInfo getSystemInfo() {
    SystemInfo info = {};
    
    // OS Info from registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
            info.osName = buffer;
        
        // Windows 11 detection: build >= 22000 is Windows 11, registry still says "Windows 10"
        size = sizeof(buffer);
        int buildNumber = 0;
        if (RegQueryValueExA(hKey, "CurrentBuildNumber", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            try { buildNumber = std::stoi(buffer); } catch (...) {}
        }
        if (buildNumber >= 22000) {
            // Replace "Windows 10" with "Windows 11" in product name
            size_t pos = info.osName.find("Windows 10");
            if (pos != std::string::npos) {
                info.osName.replace(pos, 10, "Windows 11");
            }
        }
        
        size = sizeof(buffer);
        if (RegQueryValueExA(hKey, "DisplayVersion", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
            info.osVersion = buffer;
        else {
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "ReleaseId", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
                info.osVersion = buffer;
        }
        RegCloseKey(hKey);
    }
    
    // Hostname & Username
    char hostBuf[256] = {0}, userBuf[256] = {0};
    DWORD hostSize = sizeof(hostBuf), userSize = sizeof(userBuf);
    GetComputerNameA(hostBuf, &hostSize);
    GetUserNameA(userBuf, &userSize);
    info.hostname = hostBuf;
    info.username = userBuf;
    
    // Architecture
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info.architecture = "x64"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: info.architecture = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info.architecture = "x86"; break;
        default: info.architecture = "Unknown"; break;
    }
    
    info.logicalCores = getCPULogicalCoreCount();
    info.physicalCores = getCPUPhysicalCoreCount();
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    info.totalRamGB = (float)(memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
    
    return info;
}

std::string formatSpeed(double bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes > 1024 * 1024 * 1024) ss << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB/s";
    else if (bytes > 1024 * 1024) ss << (bytes / (1024.0 * 1024.0)) << " MB/s";
    else if (bytes > 1024) ss << (bytes / 1024.0) << " KB/s";
    else ss << bytes << " B/s";
    return ss.str();
}

std::string formatBytes(long long bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes > (long long)1024 * 1024 * 1024 * 1024) ss << (bytes / (1024.0 * 1024.0 * 1024.0 * 1024.0)) << " TB";
    else if (bytes > (long long)1024 * 1024 * 1024) ss << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    else if (bytes > 1024 * 1024) ss << (bytes / (1024.0 * 1024.0)) << " MB";
    else if (bytes > 1024) ss << (bytes / 1024.0) << " KB";
    else ss << bytes << " B";
    return ss.str();
}

// ==================== MAIN PROGRAM ====================
int main(int, char**) {
    glfwSetErrorCallback([](int error, const char* description){ 
        fprintf(stderr, "GLFW Error %d: %s\n", error, description); 
    });
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 820, "Ceky Monitor v6.0 - Premium System Monitor", NULL, NULL);
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

    // Initialize data
    initCPU();
    NetStats oldNet = readNetStats();
    
    std::string cpuModel = getCPUModel();
    int cpuLogicalCores = getCPULogicalCoreCount();
    int cpuPhysicalCores = getCPUPhysicalCoreCount();
    float cpuMaxFreq = getCPUFrequency();
    SystemInfo sysInfo = getSystemInfo();
    
    float cpuUsage = 0, cpuFreq = 0;
    float ramUsage = 0, ramTotal = 0, ramAvail = 0, swapTotal = 0, swapUsed = 0, ramCommitted = 0;
    double downSpeed = 0, upSpeed = 0;
    long long totalDownloaded = 0, totalUploaded = 0;
    GPUInfo gpu = {};
    BatteryInfo battery = {};
    std::vector<DiskInfo> diskPartitions;
    DiskIOInfo diskIO = {};
    int processCount = 0, threadCount = 0, handleCount = 0;
    int activeAdapters = 0;
    std::vector<ProcessInfo> topProcesses;
    
    // History arrays (120 samples = 2 minutes)
    const int HISTORY_SIZE = 120;
    std::vector<float> cpuHistory(HISTORY_SIZE, 0), ramHistory(HISTORY_SIZE, 0);
    std::vector<float> gpuHistory(HISTORY_SIZE, 0), gpuTempHistory(HISTORY_SIZE, 0);
    std::vector<float> netDownHistory(HISTORY_SIZE, 0), netUpHistory(HISTORY_SIZE, 0);
    std::vector<float> diskReadHistory(HISTORY_SIZE, 0), diskWriteHistory(HISTORY_SIZE, 0);
    
    // Peak tracking
    float cpuPeak = 0, ramPeak = 0;
    double downPeak = 0, upPeak = 0;
    
    double lastUpdateTime = 0;
    int slowUpdateCounter = 0;
    float animTime = 0;
    
    // Tab state
    int currentTab = 0; // 0=Overview, 1=Processes, 2=System Info

    // Initial net stats for total tracking
    NetStats initialNet = readNetStats();
    long long baseRx = initialNet.rxBytes, baseTx = initialNet.txBytes;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        animTime += io.DeltaTime;

        double currentTime = glfwGetTime();
        if (currentTime - lastUpdateTime >= 1.0) {
            cpuUsage = getCPUUsage();
            cpuFreq = getCPUFrequency();
            ramUsage = getRamUsage(ramTotal, ramAvail, swapTotal, swapUsed, ramCommitted);
            
            NetStats newNet = readNetStats();
            downSpeed = (double)(newNet.rxBytes - oldNet.rxBytes);
            upSpeed = (double)(newNet.txBytes - oldNet.txBytes);
            if (downSpeed < 0) downSpeed = 0;
            if (upSpeed < 0) upSpeed = 0;
            totalDownloaded = newNet.rxBytes - baseRx;
            totalUploaded = newNet.txBytes - baseTx;
            oldNet = newNet;
            
            // Peak tracking
            if (cpuUsage > cpuPeak) cpuPeak = cpuUsage;
            if (ramUsage > ramPeak) ramPeak = ramUsage;
            if (downSpeed > downPeak) downPeak = downSpeed;
            if (upSpeed > upPeak) upPeak = upSpeed;
            
            diskIO = getDiskIO();
            
            slowUpdateCounter++;
            if (slowUpdateCounter >= 3) {
                gpu = getGPUInfo();
                battery = getBatteryInfo();
                diskPartitions = getDiskPartitions();
                processCount = getProcessCount();
                threadCount = getThreadCount();
                handleCount = getHandleCount();
                activeAdapters = getActiveNetworkAdapters();
                slowUpdateCounter = 0;
            }
            
            // Update top processes every 5 seconds
            static int procUpdateCounter = 0;
            procUpdateCounter++;
            if (procUpdateCounter >= 5) {
                topProcesses = getTopProcesses(15);
                procUpdateCounter = 0;
            }
            
            // Update histories
            auto pushHistory = [](std::vector<float>& hist, float val) {
                hist.erase(hist.begin()); hist.push_back(val);
            };
            pushHistory(cpuHistory, cpuUsage * 100);
            pushHistory(ramHistory, ramUsage * 100);
            pushHistory(gpuHistory, (float)gpu.usagePercent);
            pushHistory(gpuTempHistory, (float)gpu.temp);
            pushHistory(netDownHistory, (float)(downSpeed / 1024));
            pushHistory(netUpHistory, (float)(upSpeed / 1024));
            pushHistory(diskReadHistory, (float)(diskIO.readBytesPerSec / (1024 * 1024)));
            pushHistory(diskWriteHistory, (float)(diskIO.writeBytesPerSec / (1024 * 1024)));
            
            lastUpdateTime = currentTime;
        }

        // ==================== MODERN UI ====================
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##Main", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ===== HEADER BAR =====
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::HeaderBg);
            ImGui::BeginChild("Header", ImVec2(-1, 50), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::SetCursorPosY(8);
                
                // Animated title
                float pulse = 0.75f + 0.25f * sinf(animTime * 1.8f);
                ImVec4 titleColor = ImVec4(0.0f + 0.25f * pulse, 0.85f * pulse, 1.0f * pulse, 1.0f);
                
                ImGui::PushStyleColor(ImGuiCol_Text, titleColor);
                ImGui::SetWindowFontScale(1.4f);
                ImGui::Text("  CEKY MONITOR");
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
                ImGui::SetCursorPosY(14);
                ImGui::TextColored(Colors::TextMuted, "v6.0");
                
                // Status indicators on the right
                float rightX = ImGui::GetWindowWidth() - 520;
                ImGui::SameLine(rightX);
                ImGui::SetCursorPosY(12);
                
                // Uptime
                ImGui::TextColored(Colors::TextMuted, "UPTIME");
                ImGui::SameLine();
                ImGui::TextColored(Colors::Cyan, "%s", getUptime().c_str());
                
                ImGui::SameLine(rightX + 160);
                ImGui::TextColored(Colors::TextMuted, "PROCS");
                ImGui::SameLine();
                ImGui::TextColored(Colors::Purple, "%d", processCount);
                
                ImGui::SameLine(rightX + 260);
                ImGui::TextColored(Colors::TextMuted, "THREADS");
                ImGui::SameLine();
                ImGui::TextColored(Colors::Orange, "%d", threadCount);
                
                ImGui::SameLine(rightX + 380);
                ImGui::TextColored(Colors::TextMuted, "HANDLES");
                ImGui::SameLine();
                ImGui::TextColored(Colors::Teal, "%d", handleCount);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        
        // ===== TAB BAR =====
        {
            const char* tabNames[] = { "Overview", "Processes", "System Info" };
            ImVec4 tabColors[] = { Colors::Cyan, Colors::Purple, Colors::Teal };
            
            for (int i = 0; i < 3; i++) {
                if (i > 0) ImGui::SameLine(0, 4);
                
                bool isActive = (currentTab == i);
                if (isActive) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(tabColors[i].x * 0.2f, tabColors[i].y * 0.2f, tabColors[i].z * 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(tabColors[i].x * 0.25f, tabColors[i].y * 0.25f, tabColors[i].z * 0.25f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, tabColors[i]);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.13f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.19f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecondary);
                }
                
                if (ImGui::Button(tabNames[i], ImVec2(120, 28))) currentTab = i;
                ImGui::PopStyleColor(3);
            }
        }
        
        ImGui::Spacing();

        // ===== TAB CONTENT =====
        if (currentTab == 0) {
            // ==================== OVERVIEW TAB ====================
            float contentWidth = ImGui::GetContentRegionAvail().x;
            float col1Width = contentWidth * 0.34f;
            float col2Width = contentWidth * 0.33f;
            float col3Width = contentWidth * 0.30f;
            float spacing = 6.0f;

            // ===== COLUMN 1 =====
            ImGui::BeginChild("Col1", ImVec2(col1Width, -1), false);
            {
                // CPU Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("CPUCard", ImVec2(-1, 220), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Orange, 0.12f);
                    CardHeader("[CPU]", "PROCESSOR", Colors::Orange);
                    
                    ImGui::TextColored(Colors::TextPrimary, "%s", cpuModel.c_str());
                    ImGui::TextColored(Colors::TextSecondary, "%dC/%dT  |  %.0f MHz (base: %.0f)", 
                        cpuPhysicalCores, cpuLogicalCores, cpuFreq, cpuMaxFreq);
                    
                    ImGui::Spacing();
                    
                    ImGui::Columns(2, NULL, false);
                    ImGui::SetColumnWidth(0, 130);
                    
                    char cpuValueStr[16];
                    snprintf(cpuValueStr, sizeof(cpuValueStr), "%.0f%%", cpuUsage * 100);
                    DrawCircularProgress(cpuUsage, 48, GetUsageColor(cpuUsage), "CPU", cpuValueStr);
                    
                    ImGui::NextColumn();
                    
                    MiniGraph("##cpuGraph", cpuHistory.data(), (int)cpuHistory.size(), Colors::Orange, 55);
                    
                    ImGui::TextColored(Colors::TextMuted, "Peak:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Orange, "%.0f%%", cpuPeak * 100);
                    
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // RAM Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("RAMCard", ImVec2(-1, 200), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Green, 0.12f);
                    CardHeader("[RAM]", "MEMORY", Colors::Green);
                    
                    float usedRam = ramTotal - ramAvail;
                    char ramStr[80];
                    snprintf(ramStr, sizeof(ramStr), "%.1f / %.1f GB  (%.0f%%)", usedRam, ramTotal, ramUsage * 100);
                    ImGui::TextColored(Colors::TextPrimary, "%s", ramStr);
                    
                    ImGui::Spacing();
                    char ramOverlay[16];
                    snprintf(ramOverlay, sizeof(ramOverlay), "%.0f%%", ramUsage * 100);
                    DrawGradientProgressBar(ramUsage, ImVec2(-1, 18), Colors::Green, Colors::Cyan, ramOverlay);
                    
                    ImGui::Spacing();
                    MiniGraph("##ramGraph", ramHistory.data(), (int)ramHistory.size(), Colors::Green, 38);
                    
                    // Additional RAM details
                    ImGui::TextColored(Colors::TextMuted, "Available:");
                    ImGui::SameLine(80);
                    ImGui::TextColored(Colors::Green, "%.1f GB", ramAvail);
                    ImGui::SameLine(170);
                    ImGui::TextColored(Colors::TextMuted, "Committed:");
                    ImGui::SameLine(260);
                    ImGui::TextColored(Colors::Blue, "%.1f GB", ramCommitted);
                    
                    if (swapTotal > 0.01f) {
                        float swapPercent = (swapTotal > 0) ? (swapUsed / swapTotal) : 0;
                        ImGui::TextColored(Colors::TextMuted, "Page File:");
                        ImGui::SameLine(80);
                        ImGui::TextColored(Colors::TextSecondary, "%.1f / %.1f GB", swapUsed, swapTotal);
                        DrawGradientProgressBar(swapPercent, ImVec2(-1, 10), Colors::Purple, Colors::Pink, NULL);
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // Battery Card (if available)
                if (battery.available) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                    ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                    ImGui::BeginChild("BatCard", ImVec2(-1, 90), true, ImGuiWindowFlags_NoScrollbar);
                    {
                        ImVec4 batColor = (battery.percent < 20) ? Colors::Red : 
                                          (battery.percent < 50) ? Colors::Yellow : Colors::Green;
                        DrawCardBorderGlow(batColor, 0.12f);
                        CardHeader("[BAT]", "BATTERY", batColor);
                        
                        const char* status = battery.charging ? "Charging" : "Discharging";
                        ImGui::TextColored(batColor, "%d%%", battery.percent);
                        ImGui::SameLine();
                        ImGui::TextColored(Colors::TextSecondary, "%s", status);
                        if (battery.estimatedMinutes > 0) {
                            ImGui::SameLine();
                            int hrs = battery.estimatedMinutes / 60;
                            int mins = battery.estimatedMinutes % 60;
                            ImGui::TextColored(Colors::TextMuted, "(%dh %dm remaining)", hrs, mins);
                        }
                        
                        char batOverlay[16];
                        snprintf(batOverlay, sizeof(batOverlay), "%d%%", battery.percent);
                        DrawGradientProgressBar(battery.percent / 100.0f, ImVec2(-1, 14), batColor, Colors::Green, batOverlay);
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor(2);
                }
            }
            ImGui::EndChild();
            
            ImGui::SameLine(0, spacing);
            
            // ===== COLUMN 2 =====
            ImGui::BeginChild("Col2", ImVec2(col2Width, -1), false);
            {
                // GPU Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                float gpuCardH = gpu.available ? 220.0f : 60.0f;
                ImGui::BeginChild("GPUCard", ImVec2(-1, gpuCardH), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Purple, 0.12f);
                    CardHeader("[GPU]", "GRAPHICS", Colors::Purple);
                    
                    if (gpu.available) {
                        ImGui::TextColored(Colors::TextPrimary, "%s", gpu.name.c_str());
                        ImGui::Spacing();
                        
                        // Stats row 1
                        ImGui::Columns(3, NULL, false);
                        
                        ImVec4 tempColor = (gpu.temp > 85) ? Colors::Red : (gpu.temp > 70) ? Colors::Orange : Colors::Green;
                        ImGui::TextColored(Colors::TextMuted, "TEMP");
                        ImGui::TextColored(tempColor, "%d C", gpu.temp);
                        
                        ImGui::NextColumn();
                        ImGui::TextColored(Colors::TextMuted, "LOAD");
                        ImGui::TextColored(Colors::Purple, "%d%%", gpu.usagePercent);
                        
                        ImGui::NextColumn();
                        if (gpu.memTotalMB > 0) {
                            ImGui::TextColored(Colors::TextMuted, "VRAM");
                            ImGui::TextColored(Colors::Cyan, "%d/%dMB", gpu.memUsedMB, gpu.memTotalMB);
                        }
                        ImGui::Columns(1);
                        
                        // Stats row 2
                        if (gpu.fanSpeed >= 0 || gpu.powerDraw >= 0) {
                            ImGui::Columns(3, NULL, false);
                            if (gpu.fanSpeed >= 0) {
                                ImGui::TextColored(Colors::TextMuted, "FAN");
                                ImGui::TextColored(Colors::Teal, "%d%%", gpu.fanSpeed);
                            }
                            ImGui::NextColumn();
                            if (gpu.powerDraw >= 0) {
                                ImGui::TextColored(Colors::TextMuted, "POWER");
                                ImGui::TextColored(Colors::Amber, "%dW", gpu.powerDraw);
                            }
                            ImGui::NextColumn();
                            ImGui::Columns(1);
                        }
                        
                        ImGui::Spacing();
                        
                        // GPU usage bar
                        char gpuOverlay[16];
                        snprintf(gpuOverlay, sizeof(gpuOverlay), "%d%%", gpu.usagePercent);
                        DrawGradientProgressBar(gpu.usagePercent / 100.0f, ImVec2(-1, 14), Colors::Purple, Colors::Pink, gpuOverlay);
                        
                        // VRAM bar
                        if (gpu.memTotalMB > 0) {
                            ImGui::Spacing();
                            char vramOverlay[32];
                            snprintf(vramOverlay, sizeof(vramOverlay), "VRAM %d%%", (int)(gpu.memUsedMB * 100.0f / gpu.memTotalMB));
                            DrawGradientProgressBar(gpu.memUsedMB / (float)gpu.memTotalMB, ImVec2(-1, 10), Colors::Blue, Colors::Cyan, vramOverlay);
                        }
                        
                        ImGui::Spacing();
                        MiniGraph("##gpuGraph", gpuHistory.data(), (int)gpuHistory.size(), Colors::Purple, 30);
                    } else {
                        ImGui::TextColored(Colors::TextMuted, "No NVIDIA GPU detected");
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // Network Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("NetCard", ImVec2(-1, 220), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Cyan, 0.12f);
                    CardHeader("[NET]", "NETWORK", Colors::Cyan);
                    
                    ImGui::TextColored(Colors::TextMuted, "Adapters: %d active", activeAdapters);
                    ImGui::Spacing();
                    
                    ImGui::Columns(2, NULL, false);
                    
                    ImGui::TextColored(Colors::TextMuted, "DOWNLOAD");
                    ImGui::TextColored(Colors::Cyan, "%s", formatSpeed(downSpeed).c_str());
                    ImGui::TextColored(Colors::TextMuted, "Peak: %s", formatSpeed(downPeak).c_str());
                    MiniGraph("##netDown", netDownHistory.data(), (int)netDownHistory.size(), Colors::Cyan, 40);
                    
                    ImGui::NextColumn();
                    
                    ImGui::TextColored(Colors::TextMuted, "UPLOAD");
                    ImGui::TextColored(Colors::Pink, "%s", formatSpeed(upSpeed).c_str());
                    ImGui::TextColored(Colors::TextMuted, "Peak: %s", formatSpeed(upPeak).c_str());
                    MiniGraph("##netUp", netUpHistory.data(), (int)netUpHistory.size(), Colors::Pink, 40);
                    
                    ImGui::Columns(1);
                    
                    ImGui::Spacing();
                    ImGui::TextColored(Colors::TextMuted, "Session Total:");
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Cyan, "DL %s", formatBytes(totalDownloaded).c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(Colors::Pink, "UL %s", formatBytes(totalUploaded).c_str());
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();
            
            ImGui::SameLine(0, spacing);
            
            // ===== COLUMN 3 =====
            ImGui::BeginChild("Col3", ImVec2(col3Width, -1), false);
            {
                // Disk Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                float diskCardH = 60.0f + diskPartitions.size() * 50.0f;
                if (diskCardH > 250) diskCardH = 250;
                ImGui::BeginChild("DiskCard", ImVec2(-1, diskCardH), true);
                {
                    DrawCardBorderGlow(Colors::Yellow, 0.12f);
                    CardHeader("[HDD]", "STORAGE", Colors::Yellow);
                    
                    for (const auto& disk : diskPartitions) {
                        char diskLabel[64];
                        snprintf(diskLabel, sizeof(diskLabel), "%s  [%s]", disk.name.c_str(), disk.fileSystem.c_str());
                        ImGui::TextColored(Colors::TextPrimary, "%s", diskLabel);
                        ImGui::SameLine(140);
                        ImGui::TextColored(Colors::TextSecondary, "%.1f / %.1f GB  (%.1f GB free)", 
                            disk.usedGB, disk.totalGB, disk.freeGB);
                        
                        ImVec4 diskColor = GetUsageColor(disk.usagePercent);
                        char diskOverlay[16];
                        snprintf(diskOverlay, sizeof(diskOverlay), "%.0f%%", disk.usagePercent * 100);
                        DrawGradientProgressBar(disk.usagePercent, ImVec2(-1, 14), diskColor, diskColor, diskOverlay);
                        ImGui::Spacing();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // Disk I/O Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("DiskIOCard", ImVec2(-1, 150), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Amber, 0.12f);
                    CardHeader("[I/O]", "DISK ACTIVITY", Colors::Amber);
                    
                    ImGui::Columns(2, NULL, false);
                    
                    ImGui::TextColored(Colors::TextMuted, "READ");
                    ImGui::TextColored(Colors::Teal, "%s", formatSpeed(diskIO.readBytesPerSec).c_str());
                    MiniGraph("##diskRead", diskReadHistory.data(), (int)diskReadHistory.size(), Colors::Teal, 32);
                    
                    ImGui::NextColumn();
                    
                    ImGui::TextColored(Colors::TextMuted, "WRITE");
                    ImGui::TextColored(Colors::Orange, "%s", formatSpeed(diskIO.writeBytesPerSec).c_str());
                    MiniGraph("##diskWrite", diskWriteHistory.data(), (int)diskWriteHistory.size(), Colors::Orange, 32);
                    
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // Quick Stats Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("QuickStats", ImVec2(-1, 130), true, ImGuiWindowFlags_NoScrollbar);
                {
                    DrawCardBorderGlow(Colors::Blue, 0.12f);
                    CardHeader("[SYS]", "QUICK STATS", Colors::Blue);
                    
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%s  @  %s", sysInfo.hostname.c_str(), sysInfo.username.c_str());
                    DrawStatRow("Host", buf, Colors::Cyan, 50);
                    
                    snprintf(buf, sizeof(buf), "%s", sysInfo.osName.c_str());
                    DrawStatRow("OS", buf, Colors::Teal, 50);
                    
                    snprintf(buf, sizeof(buf), "%s (%s)", sysInfo.osVersion.c_str(), sysInfo.architecture.c_str());
                    DrawStatRow("Ver", buf, Colors::Purple, 50);
                    
                    snprintf(buf, sizeof(buf), "%.1f GB", sysInfo.totalRamGB);
                    DrawStatRow("RAM", buf, Colors::Green, 50);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();
            
        } else if (currentTab == 1) {
            // ==================== PROCESSES TAB ====================
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
            ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
            ImGui::BeginChild("ProcessList", ImVec2(-1, -1), true);
            {
                DrawCardBorderGlow(Colors::Purple, 0.10f);
                CardHeader("[TOP]", "TOP PROCESSES BY MEMORY", Colors::Purple);
                
                ImGui::TextColored(Colors::TextMuted, "Total: %d processes  |  %d threads  |  %d handles", 
                    processCount, threadCount, handleCount);
                ImGui::Spacing();
                
                // Table
                if (ImGui::BeginTable("ProcessTable", 4, 
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | 
                    ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
                    
                    ImGui::TableSetupColumn("Process", ImGuiTableColumnFlags_None, 3.0f);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_None, 1.0f);
                    ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_None, 1.5f);
                    ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_None, 1.0f);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();
                    
                    for (int i = 0; i < (int)topProcesses.size(); i++) {
                        const auto& proc = topProcesses[i];
                        ImGui::TableNextRow();
                        
                        ImGui::TableNextColumn();
                        ImVec4 nameColor = (i < 3) ? Colors::Orange : Colors::TextPrimary;
                        ImGui::TextColored(nameColor, "%s", proc.name.c_str());
                        
                        ImGui::TableNextColumn();
                        ImGui::TextColored(Colors::TextSecondary, "%lu", proc.pid);
                        
                        ImGui::TableNextColumn();
                        ImVec4 memColor = (proc.memMB > 500) ? Colors::Red : 
                                          (proc.memMB > 200) ? Colors::Orange : Colors::Green;
                        if (proc.memMB > 1024) {
                            ImGui::TextColored(memColor, "%.1f GB", proc.memMB / 1024.0f);
                        } else {
                            ImGui::TextColored(memColor, "%.0f MB", proc.memMB);
                        }
                        
                        ImGui::TableNextColumn();
                        ImGui::TextColored(Colors::TextSecondary, "%d", proc.threadCount);
                    }
                    
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor(2);
            
        } else if (currentTab == 2) {
            // ==================== SYSTEM INFO TAB ====================
            float halfW = ImGui::GetContentRegionAvail().x * 0.5f - 4;
            
            ImGui::BeginChild("SysLeft", ImVec2(halfW, -1), false);
            {
                // System Details
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("SysDetails", ImVec2(-1, 280), true);
                {
                    DrawCardBorderGlow(Colors::Teal, 0.12f);
                    CardHeader("[SYS]", "SYSTEM DETAILS", Colors::Teal);
                    
                    DrawStatRow("OS", sysInfo.osName.c_str(), Colors::TextPrimary, 120);
                    DrawStatRow("Version", sysInfo.osVersion.c_str(), Colors::TextPrimary, 120);
                    DrawStatRow("Architecture", sysInfo.architecture.c_str(), Colors::Cyan, 120);
                    DrawStatRow("Hostname", sysInfo.hostname.c_str(), Colors::Green, 120);
                    DrawStatRow("Username", sysInfo.username.c_str(), Colors::Purple, 120);
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    DrawStatRow("CPU", cpuModel.c_str(), Colors::Orange, 120);
                    char coresBuf[32];
                    snprintf(coresBuf, sizeof(coresBuf), "%d Physical / %d Logical", cpuPhysicalCores, cpuLogicalCores);
                    DrawStatRow("Cores", coresBuf, Colors::Orange, 120);
                    char freqBuf[32];
                    snprintf(freqBuf, sizeof(freqBuf), "%.0f MHz", cpuMaxFreq);
                    DrawStatRow("Base Freq", freqBuf, Colors::Orange, 120);
                    char ramBuf2[32];
                    snprintf(ramBuf2, sizeof(ramBuf2), "%.1f GB", sysInfo.totalRamGB);
                    DrawStatRow("Total RAM", ramBuf2, Colors::Green, 120);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // GPU Info
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("GPUDetails", ImVec2(-1, 160), true);
                {
                    DrawCardBorderGlow(Colors::Purple, 0.12f);
                    CardHeader("[GPU]", "GPU DETAILS", Colors::Purple);
                    
                    if (gpu.available) {
                        DrawStatRow("Name", gpu.name.c_str(), Colors::TextPrimary, 120);
                        char tempBuf[16]; snprintf(tempBuf, sizeof(tempBuf), "%d C", gpu.temp);
                        DrawStatRow("Temperature", tempBuf, (gpu.temp > 80) ? Colors::Red : Colors::Green, 120);
                        char usageBuf[16]; snprintf(usageBuf, sizeof(usageBuf), "%d%%", gpu.usagePercent);
                        DrawStatRow("GPU Load", usageBuf, Colors::Purple, 120);
                        if (gpu.memTotalMB > 0) {
                            char vramBuf[32]; snprintf(vramBuf, sizeof(vramBuf), "%d / %d MB", gpu.memUsedMB, gpu.memTotalMB);
                            DrawStatRow("VRAM", vramBuf, Colors::Cyan, 120);
                        }
                        if (gpu.fanSpeed >= 0) {
                            char fanBuf[16]; snprintf(fanBuf, sizeof(fanBuf), "%d%%", gpu.fanSpeed);
                            DrawStatRow("Fan Speed", fanBuf, Colors::Teal, 120);
                        }
                        if (gpu.powerDraw >= 0) {
                            char pwrBuf[16]; snprintf(pwrBuf, sizeof(pwrBuf), "%d W", gpu.powerDraw);
                            DrawStatRow("Power Draw", pwrBuf, Colors::Amber, 120);
                        }
                    } else {
                        ImGui::TextColored(Colors::TextMuted, "No NVIDIA GPU detected (nvidia-smi required)");
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();
            
            ImGui::SameLine(0, 8);
            
            ImGui::BeginChild("SysRight", ImVec2(halfW, -1), false);
            {
                // Performance Summary
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("PerfSummary", ImVec2(-1, 240), true);
                {
                    DrawCardBorderGlow(Colors::Cyan, 0.12f);
                    CardHeader("[PERF]", "PERFORMANCE OVERVIEW", Colors::Cyan);
                    
                    // CPU gauge
                    ImGui::TextColored(Colors::Orange, "CPU Usage");
                    ImGui::SameLine(140);
                    ImGui::TextColored(Colors::TextPrimary, "%.1f%%  (Peak: %.1f%%)", cpuUsage * 100, cpuPeak * 100);
                    DrawGradientProgressBar(cpuUsage, ImVec2(-1, 14), Colors::Orange, Colors::Red, NULL);
                    
                    ImGui::Spacing();
                    
                    // RAM gauge
                    ImGui::TextColored(Colors::Green, "RAM Usage");
                    ImGui::SameLine(140);
                    ImGui::TextColored(Colors::TextPrimary, "%.1f%%  (Peak: %.1f%%)", ramUsage * 100, ramPeak * 100);
                    DrawGradientProgressBar(ramUsage, ImVec2(-1, 14), Colors::Green, Colors::Cyan, NULL);
                    
                    ImGui::Spacing();
                    
                    // GPU gauge
                    if (gpu.available) {
                        ImGui::TextColored(Colors::Purple, "GPU Usage");
                        ImGui::SameLine(140);
                        ImGui::TextColored(Colors::TextPrimary, "%d%%", gpu.usagePercent);
                        DrawGradientProgressBar(gpu.usagePercent / 100.0f, ImVec2(-1, 14), Colors::Purple, Colors::Pink, NULL);
                        ImGui::Spacing();
                    }
                    
                    // Disk gauges
                    for (const auto& disk : diskPartitions) {
                        char lbl[32];
                        snprintf(lbl, sizeof(lbl), "Disk %s", disk.name.c_str());
                        ImGui::TextColored(Colors::Yellow, "%s", lbl);
                        ImGui::SameLine(140);
                        ImGui::TextColored(Colors::TextPrimary, "%.0f%%", disk.usagePercent * 100);
                        DrawGradientProgressBar(disk.usagePercent, ImVec2(-1, 10), 
                            GetUsageColor(disk.usagePercent), GetUsageColor(disk.usagePercent), NULL);
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                
                ImGui::Spacing();
                
                // Network Summary
                ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::CardBg);
                ImGui::PushStyleColor(ImGuiCol_Border, Colors::CardBorder);
                ImGui::BeginChild("NetSummary", ImVec2(-1, 160), true);
                {
                    DrawCardBorderGlow(Colors::Cyan, 0.12f);
                    CardHeader("[NET]", "NETWORK SUMMARY", Colors::Cyan);
                    
                    DrawStatRow("Adapters", std::to_string(activeAdapters).c_str(), Colors::Cyan, 140);
                    DrawStatRow("Current DL", formatSpeed(downSpeed).c_str(), Colors::Cyan, 140);
                    DrawStatRow("Current UL", formatSpeed(upSpeed).c_str(), Colors::Pink, 140);
                    DrawStatRow("Peak DL", formatSpeed(downPeak).c_str(), Colors::Teal, 140);
                    DrawStatRow("Peak UL", formatSpeed(upPeak).c_str(), Colors::Orange, 140);
                    DrawStatRow("Session DL", formatBytes(totalDownloaded).c_str(), Colors::Cyan, 140);
                    DrawStatRow("Session UL", formatBytes(totalUploaded).c_str(), Colors::Pink, 140);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
            }
            ImGui::EndChild();
        }

        // ===== FOOTER STATUS BAR =====
        {
            ImVec2 footerPos = ImVec2(10, io.DisplaySize.y - 22);
            ImGui::GetWindowDrawList()->AddText(footerPos, 
                IM_COL32(80, 80, 100, 255), 
                "Ceky Monitor v6.0  |  Press ESC to exit  |  Data refreshes every 1s");
        }
        
        ImGui::End();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
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

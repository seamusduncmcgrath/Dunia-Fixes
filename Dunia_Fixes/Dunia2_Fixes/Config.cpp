#include "Config.h"
#include <string>

bool g_EnableDiscordRPC = true;
bool g_EnableGrassFix = true;
bool g_EnableThreadingPatch = true;
bool g_EnableMimalloc = true;
bool g_SkipIntro = true;

uint32_t g_RenderThreadCount = 1;
uint32_t g_PhysicThreadsCount = 4;
uint32_t g_JobThreadsCount = 3;

void LoadConfig(HMODULE hModule) {
    char szPath[MAX_PATH];
    GetModuleFileNameA(hModule, szPath, MAX_PATH);

    // Figures out the INI name based off the .asi plugin name
    std::string iniPath = szPath;
    size_t lastDot = iniPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        iniPath = iniPath.substr(0, lastDot) + ".ini";
    }

    g_EnableDiscordRPC = GetPrivateProfileIntA("Main", "EnableDiscordRPC", 1, iniPath.c_str()) != 0;
    g_EnableGrassFix = GetPrivateProfileIntA("Main", "EnableGrassFix", 1, iniPath.c_str()) != 0;
    g_EnableMimalloc = GetPrivateProfileIntA("Main", "EnableMimalloc", 1, iniPath.c_str()) != 0;
    g_SkipIntro = GetPrivateProfileIntA("Main", "SkipIntro", 1, iniPath.c_str()) != 0;

    g_EnableThreadingPatch = GetPrivateProfileIntA("Threading", "EnableThreadingPatch", 1, iniPath.c_str()) != 0;
    g_RenderThreadCount = GetPrivateProfileIntA("Threading", "RenderThreadCount", 1, iniPath.c_str());
    g_PhysicThreadsCount = GetPrivateProfileIntA("Threading", "PhysicThreadsCount", 4, iniPath.c_str());
    g_JobThreadsCount = GetPrivateProfileIntA("Threading", "JobThreadsCount", 3, iniPath.c_str());

    printf("[INFO] Config loaded from: %s\n", iniPath.c_str());
}
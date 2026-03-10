#pragma once
#include <Windows.h>
#include <cstdint>

extern bool g_EnableDiscordRPC;
extern bool g_EnableGrassFix;
extern bool g_EnableThreadingPatch;
extern bool g_EnableMimalloc;
extern bool g_SkipIntro;

extern uint32_t g_RenderThreadCount;
extern uint32_t g_PhysicThreadsCount;
extern uint32_t g_JobThreadsCount;

void LoadConfig(HMODULE hModule);
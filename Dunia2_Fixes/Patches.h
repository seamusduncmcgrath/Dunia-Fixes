#pragma once
#include <Windows.h>
#include <vector>

extern uintptr_t g_RenderThread;
extern uintptr_t g_PhysicsThreads;
extern uintptr_t g_JobThreads;
extern uintptr_t g_JumpOut;

extern uint32_t RENDER_THREAD_COUNT;
extern uint32_t PHYSIC_THREADS_COUNT;
extern uint32_t JOB_THREADS_COUNT;

void ApplyDX10GrassFix(HMODULE hDunia);
bool IsLargeAddressAware();
void __stdcall ApplyThreadingPatch();
void SkipIntro(HMODULE hDunia);

void OverrideThreadingConfig_Naked();

void InstallThreadingHook(HMODULE hDunia);
uintptr_t FindMultiLevelPointer(uintptr_t baseAddress, const std::vector<unsigned int>& offsets);
bool HookIAT(const char* targetModuleName, const char* targetFunction, void* newFunction);
void InjectMimalloc();
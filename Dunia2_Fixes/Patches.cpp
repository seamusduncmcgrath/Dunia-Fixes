#include <Windows.h>
#include "MinHook.h" 
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <d3d10_1.h>
#include <d3d10.h>
#include <dxgi.h>
#include "discord_rpc.h"
#include "mimalloc.h"
#include "Graphics.h"
#include "Memory.h"
#include "Patches.h"
#include "Config.h"

// Based off jackal patch multithreading stuff, same for grass fix
// https://github.com/Tobe95/JackalPatch/tree/main
uintptr_t g_RenderThread = 0;
uintptr_t g_PhysicsThreads = 0;
uintptr_t g_JobThreads = 0;
uintptr_t g_JumpOut = 0;



bool IsLargeAddressAware() {
    HMODULE hModule = GetModuleHandleA(NULL);
    if (!hModule) return false;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

    return (pNtHeaders->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) != 0;
}

void ApplyDX10GrassFix(HMODULE hDunia) {
    uintptr_t duniaBase = (uintptr_t)hDunia;

    NopMemory(duniaBase + 0x3436D0, 6);

    PatchByte(duniaBase + 0x1607DC4, 0x03);

    printf("[SUCCESS] DX10 Grass Fix Applied!\n");
}

void SkipIntro(HMODULE hDunia) {
    uintptr_t duniaBase = (uintptr_t)hDunia;

    NopMemory(duniaBase + 0x8C735C, 9);

    printf("[SUCCESS] Intros skipped!\n");
}

void __stdcall ApplyThreadingPatch() {
    if (*(uint32_t*)(g_RenderThread - 0x28) == 0x444E4552) {
        *(uint32_t*)g_RenderThread = g_RenderThreadCount;
        *(uint32_t*)g_PhysicsThreads = g_PhysicThreadsCount;
        *(uint32_t*)g_JobThreads = g_JobThreadsCount;

        *(uint32_t*)(g_JobThreads - 0xC) = 0;         // RelativeToCoreCnt="0"
        *(uint32_t*)(g_JobThreads - 0x4) = 99;        // MaxThreadCnt="99"
        *(uint32_t*)(g_PhysicsThreads - 0x4) = 99;    // MaxThreadCnt="99"
    }
    // Checks if the game is in Benchmark Mode
    else if (*(uint32_t*)(g_RenderThread + 0x50 - 0x28) == 0x444E4552) {
        *(uint32_t*)(g_RenderThread + 0x50) = g_RenderThreadCount;
        *(uint32_t*)(g_PhysicsThreads + 0x50) = g_PhysicThreadsCount;
        *(uint32_t*)(g_JobThreads + 0x50) = g_JobThreadsCount;

        *(uint32_t*)(g_JobThreads + 0x50 - 0xC) = 0;
        *(uint32_t*)(g_JobThreads + 0x50 - 0x4) = 99;
        *(uint32_t*)(g_PhysicsThreads + 0x50 - 0x4) = 99;
    }
}

__declspec(naked) void OverrideThreadingConfig_Naked() {
    __asm {
        pushad
        call ApplyThreadingPatch
        popad

        push 0x7FFFFFFF               
        jmp[g_JumpOut]             
    }
}

void InstallThreadingHook(HMODULE hDunia) {
    uintptr_t duniaBase = (uintptr_t)hDunia;

    g_RenderThread = duniaBase + 0x80903A8;
    g_PhysicsThreads = duniaBase + 0x80903F8;
    g_JobThreads = duniaBase + 0x8090448;
    g_JumpOut = duniaBase + 0x2B2EBD;

    uintptr_t hookLocation = duniaBase + 0x2B2EB8;

    CreateJmp(hookLocation, &OverrideThreadingConfig_Naked);

    printf("[SUCCESS][EXPERIMENTAL] CPU Multithreading improvments applied!\n");
}

bool HookIAT(const char* targetModuleName, const char* targetFunction, void* newFunction) {
    HMODULE hModule = GetModuleHandleA(targetModuleName);
    if (!hModule) return false;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);

    IMAGE_DATA_DIRECTORY importDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDirectory.Size == 0) return false;

    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule + importDirectory.VirtualAddress);

    while (importDesc->Name) {
        const char* libraryName = (const char*)((BYTE*)hModule + importDesc->Name);

        char lowerLibName[256];
        strcpy_s(lowerLibName, sizeof(lowerLibName), libraryName);
        _strlwr_s(lowerLibName);

        if (strstr(lowerLibName, "msvcr") != nullptr) {
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDesc->FirstThunk);
            PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDesc->OriginalFirstThunk);

            while (origThunk->u1.AddressOfData) {
                if (!IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) {
                    PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hModule + origThunk->u1.AddressOfData);

                    if (strcmp(importByName->Name, targetFunction) == 0) {
                        DWORD oldProtect;
                        VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR), PAGE_READWRITE, &oldProtect);
                        thunk->u1.Function = (ULONG_PTR)newFunction;
                        VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR), oldProtect, &oldProtect);
                        return true;
                    }
                }
                thunk++;
                origThunk++;
            }
        }
        importDesc++;
    }
    return false;
}

// Swaps malloc for mimalloc
void InjectMimalloc() {
    printf("[INFO] Scanning IAT and injecting mimalloc...\n");

    int hooksPlaced = 0;

    if (HookIAT("dunia.dll", "malloc", &mi_malloc)) hooksPlaced++;
    if (HookIAT("dunia.dll", "free", &mi_free)) hooksPlaced++;
    if (HookIAT("dunia.dll", "calloc", &mi_calloc)) hooksPlaced++;
    if (HookIAT("dunia.dll", "realloc", &mi_realloc)) hooksPlaced++;

    if (hooksPlaced > 0) {
        printf("[SUCCESS] Dunia memory stuff patched to mimalloc! (%d functions routed to mimalloc)\n", hooksPlaced);
    }
    else {
        printf("[FAILED] Could not find memory functions in the IAT.\n");
    }
}


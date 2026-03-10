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

void PatchByte(uintptr_t address, uint8_t value) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
    *(uint8_t*)address = value;
    VirtualProtect((LPVOID)address, 1, oldProtect, &oldProtect);
}

void CreateJmp(uintptr_t address, void* destination) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    *(uint8_t*)address = 0xE9; 
    *(uintptr_t*)(address + 1) = (uintptr_t)destination - address - 5;

    VirtualProtect((LPVOID)address, 5, oldProtect, &oldProtect);
}

void NopMemory(uintptr_t address, size_t size) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memset((LPVOID)address, 0x90, size);
    VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
}

uintptr_t FindMultiLevelPointer(uintptr_t baseAddress, const std::vector<unsigned int>& offsets) {
    uintptr_t currentAddress = baseAddress;

    __try {
        for (size_t i = 0; i < offsets.size(); ++i) {
            currentAddress = *(uintptr_t*)currentAddress;
            if (currentAddress == 0) return 0;
            currentAddress += offsets[i];
        }
        return currentAddress;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}
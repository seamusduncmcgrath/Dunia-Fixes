#pragma once

#include <Windows.h>
#include <vector>
#include <cstdint>

void PatchByte(uintptr_t address, uint8_t value);
void NopMemory(uintptr_t address, size_t size);
uintptr_t FindMultiLevelPointer(uintptr_t baseAddress, const std::vector<unsigned int>& offsets);
void CreateJmp(uintptr_t address, void* destination);
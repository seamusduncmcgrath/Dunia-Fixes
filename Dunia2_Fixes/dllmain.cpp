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

int g_LastAmmo = -1;

// discord RPC thread
DWORD WINAPI RPCWorkerThread(LPVOID lpParam) {
    printf("Discord RPC started\n\n");

    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize("1480422766405615666", &handlers, 1, NULL);
    printf("[SUCCESS] Connected to discord\n");

    //Wait safely until dunia.dll is fully loaded
    HMODULE hDunia = NULL;
    while (!hDunia) {
        hDunia = GetModuleHandleA("dunia.dll");
        Sleep(100);
    }
    printf("[SUCCESS] dunia.dll found\n\n");

    if (g_SkipIntro) {
        SkipIntro(hDunia);
    }

    if (g_EnableMimalloc) {
        InjectMimalloc();
    }
    if (g_EnableGrassFix) {
        ApplyDX10GrassFix(hDunia);
    }
    if (g_EnableThreadingPatch) {
        InstallThreadingHook(hDunia);
    }

    uintptr_t baseAddress = (uintptr_t)hDunia + 0x0160577C;
    std::vector<unsigned int> ammoOffsets = { 0x18, 0x38, 0xAC, 0x44, 0xF8, 0x1C, 0x98 };

    while (true) {
        uintptr_t finalAmmoAddr = FindMultiLevelPointer(baseAddress, ammoOffsets);

        if (finalAmmoAddr != 0) {
            int currentAmmo = *(int*)finalAmmoAddr;

            if (currentAmmo != g_LastAmmo) {
                printf("Ammo changed to %d -> Updating Discord...\n", currentAmmo);

                DiscordRichPresence discordPresence;
                memset(&discordPresence, 0, sizeof(discordPresence));

                std::string ammoString = "Ammo: " + std::to_string(currentAmmo);

                discordPresence.state = ammoString.c_str();
                discordPresence.details = "Exploring Africa";
                discordPresence.largeImageKey = "fc2_main"; 
                discordPresence.largeImageText = "Far Cry 2";

                Discord_UpdatePresence(&discordPresence);

                g_LastAmmo = currentAmmo;
            }
        }
        else {
            // Pretty sure this breaks something when you change your loadout
            if (g_LastAmmo != -2) {
                printf("Weapon unequipped. Updating Discord to Menu State...\n");

                DiscordRichPresence discordPresence;
                memset(&discordPresence, 0, sizeof(discordPresence));

                discordPresence.state = "In Menus / Safezone";
                discordPresence.details = "Exploring Africa";
                discordPresence.largeImageKey = "fc2_main";

                Discord_UpdatePresence(&discordPresence);
                g_LastAmmo = -2;
            }
        }

        Discord_RunCallbacks();
        Sleep(2000);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);

        LoadConfig(hModule);

#ifdef _DEBUG
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        printf("Dunia Fixes Starting\n\n");

#endif

        if (g_EnableDiscordRPC) {
            CreateThread(NULL, 0, RPCWorkerThread, NULL, 0, NULL);
        }

        if (!IsLargeAddressAware()) {
            MessageBoxA(
                NULL,
                "Far Cry 2 is not Large Address Aware (LAA).\n\n"
                "Please apply a 4GB patch to farcry2.exe. Without this, the game is limited to 2GB of RAM and may crash during extended play or when using complex mods.",
                "Dunia Fixes - Memory Warning",
                MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL
            );
        }
        if (MH_Initialize() == MH_OK) {
            MH_CreateHook(&CreateWindowExA, &HookedCreateWindowExA, reinterpret_cast<LPVOID*>(&OriginalCreateWindowExA));
            MH_EnableHook(MH_ALL_HOOKS);
        }
        break;
    }
    case DLL_PROCESS_DETACH: {
        Discord_Shutdown();
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        break;
    }
    }
    return TRUE;
}
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


typedef HWND(WINAPI* CreateWindowExA_t)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
CreateWindowExA_t OriginalCreateWindowExA = nullptr;

// Stuff for borderless windowed
HWND WINAPI HookedCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    if (hWndParent == NULL) {
        dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        dwStyle |= WS_POPUP;
        X = 0;
        Y = 0;
        nWidth = GetSystemMetrics(SM_CXSCREEN);
        nHeight = GetSystemMetrics(SM_CYSCREEN);
        static bool bIsDXGIHooked = false;
        if (!bIsDXGIHooked) {
            printf("[INFO] Main window created. Attempting to mod DX10...\n");
            HookDXGI();
            bIsDXGIHooked = true;
        }
    }

    return OriginalCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

const GUID IID_IDXGIFactory_Custom = { 0x7b7166ec, 0x21c7, 0x44ae, { 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69 } };


// Should fix colour banding issues when in fullscreen mode
typedef HRESULT(WINAPI* CreateSwapChain_t)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**);
CreateSwapChain_t OriginalCreateSwapChain = nullptr;

HRESULT WINAPI HookedCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) {
    if (pDesc != nullptr) {
        printf("[DXGI Hook] Intercepted SwapChain! Forcing 32-bit Uncompressed Color...\n");
        pDesc->BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    return OriginalCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
}

// Hooks DXGI as the name suggests
void HookDXGI() {
    HMODULE hDXGI = LoadLibraryA("dxgi.dll");

    if (hDXGI) {
        typedef HRESULT(WINAPI* CreateDXGIFactory_t)(REFIID, void**);
        auto pCreateDXGIFactory = (CreateDXGIFactory_t)GetProcAddress(hDXGI, "CreateDXGIFactory");

        if (pCreateDXGIFactory) {
            IDXGIFactory* pDummyFactory = nullptr;

            if (SUCCEEDED(pCreateDXGIFactory(IID_IDXGIFactory_Custom, (void**)&pDummyFactory))) {
                void** pVTable = *reinterpret_cast<void***>(pDummyFactory);
                void* pCreateSwapChainAddr = pVTable[10];

                if (MH_CreateHook(pCreateSwapChainAddr, &HookedCreateSwapChain, reinterpret_cast<LPVOID*>(&OriginalCreateSwapChain)) == MH_OK) {
                    MH_EnableHook(pCreateSwapChainAddr);
                    printf("[SUCCESS] DXGI pipeline hooked!\n");
                }

                pDummyFactory->Release();
            }
        }
    }
}
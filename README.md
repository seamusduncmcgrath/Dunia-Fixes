Far Cry 2 Modernized
A comprehensive ASI plugin for Far Cry 2 that overhauls the engine's legacy memory management, optimizes CPU multithreading, fixes long-standing graphical bugs, and adds modern quality-of-life features.

Features
Performance & Engine Overhauls
Next-Gen Memory Management: Completely intercepts and replaces the 2008 Dunia engine memory allocator with Microsoft's mimalloc. This dramatically reduces stuttering, improves 1% low frame pacing, and decreases load times by un-locking map chunk streaming.

CPU Multithreading Optimization: Injects a dynamic mid-function hook to override the game's default threading configuration. This un-bottlenecks the main render thread and properly distributes physics and engine jobs across modern multi-core CPUs.

Visual & Bug Fixes
DX10 Grass Fix: Restores the proper height and density of grass when running the game in DirectX 10 mode, fixing a decade-old rendering bug.

Color Banding Fix: Intercepts the DXGI SwapChain creation to force 32-bit uncompressed color, significantly reducing the harsh color banding present in the vanilla game's skybox and shadows.

Quality of Life
Skip Intro Movies: Bypasses the unskippable publisher logos on boot, dropping you instantly into the main menu.

Borderless Windowed Mode: Forces a true borderless windowed mode for modern monitors and seamless Alt-Tabbing.

Discord Rich Presence: Real-time Discord integration that dynamically tracks and displays your current ammo count and menu/exploration states.

Installation
This mod requires an ASI loader to inject into the game.

Download the 32-bit (x86) version of Ultimate ASI Loader.

Extract the downloaded file and rename it to dinput8.dll.

Download the latest release of Far Cry 2 Modernized.

Place dinput8.dll, your .asi plugin, and the .ini configuration file directly into your Far Cry 2 bin folder (where farcry2.exe is located).

Launch the game.

Configuration
The mod includes a configuration file that allows you to toggle individual features on or off. Open the .ini file in any text editor to customize your experience.

Ini, TOML
[Main]
EnableDiscordRPC=1
EnableGrassFix=1
EnableMimalloc=1
SkipIntroMovies=1
EnableDebugConsole=0

[Threading]
EnableThreadingPatch=1
RenderThreadCount=1
PhysicThreadsCount=4
JobThreadsCount=3
Note: Set EnableDebugConsole=1 if you wish to see the background injection and hooking processes during gameplay.

Credits & Acknowledgments
Microsoft for the mimalloc memory allocator.

TsudaKageyu for the MinHook library.

The Far Cry 2 modding community and open-source contributors who discovered the memory offsets for the DX10 Grass Fix, Intro Skip, and the raw Threading Configuration logic used in this plugin.

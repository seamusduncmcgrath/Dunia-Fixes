# Dunia Fixes

A comprehensive ASI plugin for Far Cry 2 that overhauls the engine's legacy memory management, optimizes CPU multithreading, fixes long-standing graphical bugs, and adds modern quality-of-life features.

## Features

### Performance & Engine Overhauls
* **Next-Gen Memory Management:** Completely intercepts and replaces the 2008 Dunia engine memory allocator with Microsoft's `mimalloc`. This dramatically reduces stuttering, improves 1% low frame pacing, and decreases load times by un-locking map chunk streaming.
* **CPU Multithreading Optimization:** Overrides the game's default threading configuration. This un-bottlenecks the main render thread and properly distributes physics and engine jobs across modern multi-core CPUs (Experimental, can reduce performance.

### Visual & Bug Fixes
* **DX10 Grass Fix:** Restores the proper height and density of grass when running the game in DirectX 10 mode, fixing a decade-old rendering bug.
* **Color Banding Fix:** Intercepts the DXGI SwapChain creation to force 32-bit uncompressed color, significantly reducing the harsh color banding present in the vanilla game's skybox and shadows.

### Quality of Life
* **Skip Intro Movies:** Bypasses the unskippable publisher logos on boot, dropping you instantly into the main menu.
* **Borderless Windowed Mode:** Forces a true borderless windowed mode for modern monitors and seamless Alt-Tabbing.
* **Discord Rich Presence:** Real-time Discord integration that dynamically tracks and displays your current ammo count and menu/exploration states.

## Installation

This mod requires an ASI loader to inject into the game.

1. Download the **32-bit (x86)** version of [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).
2. Extract the downloaded file and rename it to `dinput8.dll`.
3. Download the latest release of **Far Cry 2 Modernized**.
4. Place `dinput8.dll`, your `.asi` plugin, and the `.ini` configuration file directly into your Far Cry 2 `bin` folder (where `farcry2.exe` is located).
5. Launch the game.

## Configuration

The mod includes a basic configuration file that allows you to toggle individual features on or off.

```ini
[Main]
EnableDiscordRPC=1
EnableGrassFix=1
EnableMimalloc=1
SkipIntro=1

[Threading]
EnableThreadingPatch=1
RenderThreadCount=1
PhysicThreadsCount=4
JobThreadsCount=3
```
*Note: The improved multithreading is experimental and as a result disabled by default.*

## Credits & Acknowledgments
* Microsoft for the `mimalloc` memory allocator.
* TsudaKageyu for the `MinHook` library.
* Tobe95 for his DX10 grass fix & muiltithreading from JackalPatch.

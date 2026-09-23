# My Shooter

A 60-second top-down arcade shooter written in C++20 with [raylib](https://www.raylib.com/). The whole game lives in one translation unit: a small state machine, frame-time movement, AABB hits, and a menu / play / game-over loop.

Built as a Visual Studio 2022 (v143) Win32 project. Language is reported as C++ on GitHub because that is the only source language in the tree.

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C)
![raylib 6.0](https://img.shields.io/badge/raylib-6.0-000000)
![VS 2022](https://img.shields.io/badge/Visual%20Studio-2022-5C2D91)

## What you play

Survive one minute on a 1200×1000 field. Enemies drop in from the top, last six seconds or until they leave the bottom, and never more than three are alive at once. Each hit is worth 10 points. Game over shows score and kill ratio (`killed / spawned`).

| Control | Action |
|---|---|
| `W` `A` `S` `D` | Move (diagonals are normalized, so they are not faster) |
| Left mouse | Fire |
| Left stick (gamepad 0) | Move, 0.2 analog deadzone |
| Menu / Game Over buttons | Start, Play Again, Exit |

## How the code is structured

`Myshooter/main.cpp` is split into short functions around four types:

```
GameState  MENU → PLAYING → GAMEOVER → EXIT
Player     position, speed, size
Bullet     position, speed, active
Enemy      position, speed, size, active, lifeTimer
```

- **Loop.** `GetFrameTime()` is the only clock. Playing subtracts from a 60s timer, then updates player, bullets, enemies, collisions, and dead-entity cleanup before drawing.
- **Input.** Keyboard axes are collected into a `Vector2`, then `Vector2Normalize`. A connected pad overwrites that vector when the stick is outside the deadzone.
- **World bounds.** Player is clamped to `[0, 1200 − size]` × `[0, 1000 − size]`.
- **Spawning.** Every 1.5s, if fewer than three enemies exist, one is placed at a random x and `y = −50`.
- **Hits.** Bullet 5×10 vs enemy square, `CheckCollisionRecs`. Both objects are marked inactive; score and kill count go up.
- **Lifetime.** Inactive bullets and enemies are removed with `std::remove_if` so the vectors do not grow for the whole round.
- **HUD.** Score, `killed/total`, and `mm:ss`. The timer turns red in the last ten seconds. Enemy HP bars are remaining lifetime.

Vectors are reserved up front (`200` bullets, `20` enemies) so a busy minute does not reallocate on the hot path.

```
while window open and not EXIT
    dt = GetFrameTime()
    if PLAYING
        tick timer → GAMEOVER at 0
        UpdatePlayer / UpdateBullets / UpdateEnemies
        CheckCollisions
        CleanupDead
    draw MENU | PLAYING | GAMEOVER
```

## Layout

```
Myshooter.sln                 solution (VS 17)
Myshooter/main.cpp            game
Myshooter/Myshooter.vcxproj   MSBuild, x64 Debug and Release
.gitignore                    Visual Studio ignores
.gitattributes
```

No extra source files, assets, or scripts are in this repository. Build against a local raylib 6.0 MSVC 64-bit install.

## Build (Windows)

Need:

- Visual Studio 2022 with the Desktop C++ workload (toolset v143, Windows 10 SDK)
- [raylib 6.0 Windows MSVC](https://github.com/raysan5/raylib/releases) unpacked somewhere you can point at

The project looks for raylib here by default:

```
C:\code\libs\raylib-6.0_win64_msvc16
```

That directory should contain `include/` (`raylib.h`, `raymath.h`) and `lib/raylib.lib`. If yours is elsewhere, set a user or system environment variable before opening the solution:

```
RAYLIB_DIR=D:\libs\raylib-6.0_win64_msvc16
```

Then:

1. Open `Myshooter.sln`
2. Configuration: **Release | x64** (or **Debug | x64**)
3. Build and run (`F5`)

Link line is `raylib.lib`, `opengl32.lib`, `gdi32.lib`, `winmm.lib`, `shell32.lib`. C++ language standard is `/std:c++20`. Win32 (x86) configurations exist in the solution file but are not wired to raylib; use x64.

## License

Source in this repo is provided as-is for viewing and building. raylib has its own zlib-style license; follow that for the library binary you link.

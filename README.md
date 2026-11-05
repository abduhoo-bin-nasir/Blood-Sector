<div align="center">

<img src="assets/banner.svg" alt="BLOOD SECTOR: a retro first-person horror shooter, raycasted in C++ with Raylib" width="100%">

<br>

<img src="https://img.shields.io/badge/C%2B%2B17-120808?style=for-the-badge&logo=cplusplus&logoColor=e9b44c" alt="C++17">
<img src="https://img.shields.io/badge/Raylib_5.x-120808?style=for-the-badge&logoColor=e9b44c" alt="Raylib 5.x">
<img src="https://img.shields.io/badge/CMake-120808?style=for-the-badge&logo=cmake&logoColor=e9b44c" alt="CMake">
<img src="https://img.shields.io/badge/Windows_%7C_Linux_%7C_macOS-120808?style=for-the-badge&logoColor=e9b44c" alt="Platforms: Windows, Linux, macOS">
<a href="#-license"><img src="https://img.shields.io/badge/License-MIT-120808?style=for-the-badge&labelColor=120808&color=d62828" alt="MIT License"></a>

</div>

<br>

<img src="assets/sec-briefing.svg" alt="01 Briefing" width="100%">

> *"No engine. No mercy. Just rays."*

**Blood Sector** is a retro-inspired first-person horror shooter built in **C++** with **Raylib** and classic **raycasting**. Inspired by early FPS games like Doom, it renders a pseudo-3D world entirely through a custom raycaster, with no game engine underneath.

Fight through four sectors, learn how each monster hunts you, scavenge ammo, and survive a boss at the bottom of it all.

<img src="assets/stats.svg" alt="4 sectors, 3 enemy types, a 20 HP boss, 0 game engines" width="100%">

<br>

<p align="center">
  <img src="assets/feature-raycast.svg" alt="Raycast renderer: a Doom-style pseudo-3D world drawn by a custom DDA raycaster, no game engine involved." width="49%">
  <img src="assets/feature-ai.svg" alt="Enemy AI: Walkers patrol and chase, Stalkers move only when unobserved, the Boss charges and summons Walkers." width="49%">
  <img src="assets/feature-campaign.svg" alt="Four sectors: a progressive multi-level campaign from The Breach to The Core, ending in a boss battle." width="49%">
  <img src="assets/feature-sprites.svg" alt="Dynamic sprites: enemies are projected into the 3D view as sprites that scale with distance." width="49%">
  <img src="assets/feature-minimap.svg" alt="Minimap and ammo: toggle the minimap with TAB, scavenge ammo pickups and manage your resources." width="49%">
  <img src="assets/feature-sound.svg" alt="Sound and portable: sound effects, background music and a cross-platform CMake build." width="49%">
</p>

<br>

<img src="assets/sec-footage.svg" alt="02 Footage" width="100%">

<p align="center">
  <img src="assets/screenshots/gameplay1.png" width="49%" alt="Blood Sector gameplay screenshot">
  <img src="assets/screenshots/gameplay2.png" width="49%" alt="Blood Sector gameplay screenshot">
  <br>
  <sub><i>Figure 1: Real in-engine captures from the raycaster.</i></sub>
</p>

<br>

<img src="assets/sec-bestiary.svg" alt="03 Bestiary" width="100%">

Three kinds of things want you dead. Each one needs a different answer.

<img src="assets/bestiary.svg" alt="Bestiary: Walker with 3 HP patrols, chases and attacks in melee. Stalker with 5 HP moves only when not observed. Boss with 20 HP charges and summons Walkers." width="100%">

<sub><i>Sprites and state chips above are redrawn illustrations of the behavior, not in-game assets.</i></sub>

| Enemy | Health | Behavior |
| :--- | :---: | :--- |
| **Walker** | 3 HP | Random patrol, chase, melee attack |
| **Stalker** | 5 HP | Moves only when not being observed |
| **Boss** | 20 HP | Charge attacks and summons Walkers |

<br>

<img src="assets/sec-sectors.svg" alt="04 Sectors" width="100%">

<img src="assets/sectors.svg" alt="The four sectors: The Breach, The Corridor, The Labyrinth, The Core." width="100%">

<sub><i>The layouts above are simplified sketches of each sector's character, not the real level maps.</i></sub>

| Sector | Description |
| :--- | :--- |
| **The Breach** | Introduction to core mechanics |
| **The Corridor** | Tight spaces and ambush encounters |
| **The Labyrinth** | Maze-like navigation challenge |
| **The Core** | Final boss arena |

<br>

<img src="assets/sec-controls.svg" alt="05 Controls" width="100%">

<img src="assets/controls.svg" alt="Controls: W A S D to move, mouse to look, left click or space to shoot, TAB for the minimap, R to restart the level, ESC to exit." width="100%">

| Key | Action |
| :--- | :--- |
| `W` `A` `S` `D` | Move |
| `Mouse` | Look around |
| `Left Click` / `Space` | Shoot |
| `TAB` | Toggle minimap |
| `R` | Restart level |
| `ESC` | Exit game |

<br>

<img src="assets/sec-engine.svg" alt="06 Engine" width="100%">

Every frame is built from scratch. A ray is cast for each column of the screen, a **DDA** walks the grid from line to line until it hits a wall, and the hit distance decides how tall that wall slice is drawn. Sprites are then projected on top.

<img src="assets/raycast-dda.svg" alt="Diagram of the raycasting loop: a top-down map with rays and DDA steps on the left, and the resulting wall columns on the right." width="100%">

<sub><i>A simplified illustration of the technique, not a capture from the game.</i></sub>

The code is split into small, object-oriented modules built around an abstract `Entity` base class:

<img src="assets/architecture.svg" alt="Simplified module layout: main starts Game, which drives Renderer, Map and LevelManager and owns an Entity hierarchy with Player and Enemy, where Enemy has Walker, Stalker and Boss variants." width="100%">

<sub><i>Simplified relationships between the modules. See <code>src/</code> for the real thing.</i></sub>

| Technique | What it does here |
| :--- | :--- |
| **Object-oriented design** | Game, renderer, map, player, enemies and levels live in separate classes |
| **Inheritance and polymorphism** | Player and enemies share an abstract `Entity` interface |
| **Dynamic enemy management** | Enemies are created, updated and removed as the level plays out |
| **DDA raycasting** | Finds the first wall hit for every screen column |
| **Sprite projection** | Places enemies and pickups into the 3D view |
| **Collision detection** | Keeps the player and enemies out of walls |
| **State-based architecture** | The game flow is driven by explicit states |

<br>

<img src="assets/sec-build.svg" alt="07 Build" width="100%">

**Requirements:** a C++17 compatible compiler and CMake 3.15+.

```bash
# Clone the repository
git clone https://github.com/abduhoo-bin-nasir/Blood-Sector.git
cd Blood-Sector
```

**Configure**

```bash
# Windows (MinGW)
cmake -B build -G "MinGW Makefiles"

# Linux / macOS
cmake -B build
```

**Build**

```bash
cmake --build build
```

**Run**

```bash
# Windows
./build/Blood-Sector.exe

# Linux / macOS
./build/Blood-Sector
```

> [!NOTE]
> Copy the `assets/` folder into the build directory before running the executable.

<details>
<summary><b>Project structure</b></summary>

```text
blood-sector/
│
├── src/
│   ├── main.cpp
│   ├── Game.h / Game.cpp
│   ├── Renderer.h / Renderer.cpp
│   ├── Player.h / Player.cpp
│   ├── Enemy.h / Enemy.cpp
│   ├── Entity.h / Entity.cpp
│   ├── Map.h / Map.cpp
│   └── LevelManager.h / LevelManager.cpp
│
├── assets/
│   ├── sprites/
│   ├── sounds/
│   ├── music/
│   └── screenshots/
│
├── CMakeLists.txt
├── README.md
└── BUILD.md
```

</details>

<br>

## 📜 License

This project is licensed under the MIT License.

<br>

<img src="assets/footer.svg" alt="You died. Press R to restart the level." width="100%">

<p align="center"><sub>Built by <a href="https://abduhoo-bin-nasir.github.io">Abduhoo Bin Nasir</a> &nbsp;·&nbsp; <a href="https://github.com/abduhoo-bin-nasir">GitHub</a></sub></p>

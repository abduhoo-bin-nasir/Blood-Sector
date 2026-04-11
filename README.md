# 🩸 Blood Sector

A first-person horror raycasting game built in C++ with Raylib.  
Inspired by classic Doom-era FPS games — no engine, no shortcuts, just raw raycasting math and OOP.

![C++](https://img.shields.io/badge/C++-17-blue?style=flat-square&logo=cplusplus)
![Raylib](https://img.shields.io/badge/Raylib-5.x-red?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Mac-lightgrey?style=flat-square)

---

## 🎮 Gameplay

You are trapped inside a breached facility. Four sectors. No way out except through.

- Shoot enemies before they reach you
- Watch your ammo — a drop spawns on the map when you run out
- Reach the next sector by killing every enemy
- Survive the boss in Sector 4

---

## 👾 Enemy Types

| Enemy | HP | Behaviour |
|---|---|---|
| **Walker** | 3 | Patrols randomly, chases on sight, attacks in melee |
| **Stalker** | 5 | Freezes completely when you look at it — rushes the moment you look away |
| **Boss** | 20 | Slow stalker that winds up a charge lunge. Spawns additional Walkers mid-fight |

---

## 🗺️ Levels

| Sector | Layout | Theme |
|---|---|---|
| 1 — The Breach | Open chaotic rooms | Tutorial — learn the enemies |
| 2 — The Corridor | Two rooms + connecting corridor | Ambushes in tight spaces |
| 3 — The Labyrinth | Zigzag snake path | No room to run |
| 4 — The Core | Large open arena | Boss fight |

---

## 🕹️ Controls

| Key | Action |
|---|---|
| `W A S D` | Move |
| `Mouse` | Look |
| `Arrow Keys` | Look (alternative) |
| `Left Click` / `Space` | Shoot |
| `TAB` | Toggle minimap |
| `R` | Restart |
| `ESC` | Quit |

---

## 🔧 Building from Source

### Prerequisites
- g++ with C++17 support (MinGW-w64 on Windows)
- CMake 3.15+
- [Raylib](https://github.com/raysan5/raylib/releases) installed

### Steps

```bash
# Clone the repo
git clone https://github.com/yourusername/blood-sector.git
cd blood-sector

# Configure
cmake -B build -G "MinGW Makefiles"   # Windows
cmake -B build                         # Linux / Mac

# Build
cmake --build build

# Run
./build/BloodSector.exe   # Windows
./build/BloodSector        # Linux / Mac
```

> **Note:** Copy the `assets/` folder next to the executable after building, otherwise sprites and audio will not load.

---

## 📁 Project Structure

```
blood-sector/
├── src/
│   ├── main.cpp           Entry point
│   ├── Entity.h/.cpp      Abstract base class — position, health, collision
│   ├── Map.h/.cpp         Tile grid, wall queries
│   ├── Player.h/.cpp      Input handling, raycasting vectors, hitscan shooting
│   ├── Enemy.h/.cpp       Enemy base class + Walker + Stalker + Boss
│   ├── Renderer.h/.cpp    DDA raycasting, sprite projection, HUD, minimap
│   ├── Game.h/.cpp        Game loop, state machine, level transitions
│   └── LevelManager.h/.cpp  Level layouts, spawn tables, level metadata
├── assets/
│   ├── sprites/           PNG sprite sheets (place next to exe)
│   ├── sounds/            WAV sound effects (optional)
│   └── music/             MP3 ambient track (optional)
└── CMakeLists.txt
```

---

## 🧱 OOP Architecture

The project demonstrates core Object-Oriented Programming concepts:

**Inheritance**  
`Player` and `Enemy` both inherit from `Entity` (shared position, health, collision).  
`Walker`, `Stalker`, and `Boss` all inherit from `Enemy`.

**Polymorphism**  
Enemies are stored as `vector<Enemy*>`. Calling `e->behavior()` automatically dispatches to the correct subclass at runtime — no type checking needed.

**Abstraction**  
`Entity` and `Enemy` are abstract classes with pure virtual methods. You cannot instantiate them directly — only their concrete subclasses.

**Encapsulation**  
`Map`'s grid array is private. `Player`'s cooldown timers are private. All access goes through clean public methods.

**Separation of Concerns**  
`Renderer` knows nothing about game logic. `Game` never calls `DrawRectangle`. `LevelManager` only knows about levels. Each class has one job.

---

## 🎨 Assets

Audio and sprites are optional — the game runs fully without them using procedural rendering and silence as fallbacks.

Free asset sources used:
- [freesound.org](https://freesound.org) — sound effects
- [Piskel](https://piskel.com) — pixel art sprites
- [incompetech.com](https://incompetech.com) — ambient music

---

## 📜 License

MIT License — free to use, modify, and distribute.

# SHADOW BREACH — Build Instructions

## Prerequisites
- g++ with C++17 support (MinGW-w64 on Windows)
- CMake 3.15+
- Raylib installed (you already have this from the blockchain project)

---

## Build Steps (Windows — MinGW)

Open a terminal in the ShadowBreach/ folder, then:

```bash
# 1. Configure
cmake -B build -G "MinGW Makefiles"

# 2. Compile
cmake --build build

# 3. Run
./build/ShadowBreach.exe
```

---

## If raylib isn't found automatically

Open CMakeLists.txt and update these two lines with your actual raylib path:

```
"C:/raylib/lib"          ← path to the folder containing libraylib.a
"C:/raylib/include"      ← path to the folder containing raylib.h
```

---

## Build Steps (Linux/Mac)

```bash
cmake -B build
cmake --build build
./build/ShadowBreach
```

---

## Controls

| Key               | Action              |
|-------------------|---------------------|
| W / Up Arrow      | Move forward        |
| S / Down Arrow    | Move backward       |
| A                 | Strafe left         |
| D                 | Strafe right        |
| Left / Right Arrow| Rotate              |
| Mouse             | Look (FPP mode)     |
| Space / Left Click| Shoot               |
| TAB               | Switch FPP ↔ TPP    |
| R                 | Restart (after death)|
| ESC               | Quit                |

---

## Enemy Types

| Name    | HP | Behaviour                                              |
|---------|----|--------------------------------------------------------|
| Walker  | 3  | Patrols, chases on sight, attacks in melee             |
| Stalker | 5  | Freezes when you look at it — rushes when you look away|

---

## File Structure (for viva reference)

```
ShadowBreach/
├── CMakeLists.txt
└── src/
    ├── main.cpp         Entry point
    ├── Entity.h/.cpp    Abstract base class (position, health, collision)
    ├── Map.h/.cpp       Tile grid, wall queries
    ├── Player.h/.cpp    Input, raycasting vectors, hitscan shooting
    ├── Enemy.h/.cpp     Enemy base class + Walker + Stalker (polymorphism)
    ├── Renderer.h/.cpp  FPP raycasting + TPP top-down drawing
    └── Game.h/.cpp      Owns everything, runs the loop
```

## OOP Concepts Present (viva checklist)

- **Inheritance**: Player and Enemy both inherit Entity. Walker and Stalker inherit Enemy.
- **Polymorphism**: `vector<Enemy*>` — calling `e->behavior()` runs Walker's or Stalker's version automatically.
- **Abstraction**: Entity and Enemy are abstract (pure virtual methods) — you can't create one directly.
- **Encapsulation**: Map's grid is private. Player's cooldowns are private. Only public methods exposed.
- **Virtual destructor**: Entity has `virtual ~Entity()` so deleting through base pointer works correctly.
- **Separation of concerns**: Renderer knows nothing about game logic. Game knows nothing about drawing.

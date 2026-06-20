# Blood Sector — Complete Viva Preparation Guide
### Everything you need. Nothing you don't.

---

# HOW TO USE THIS DOCUMENT

- **Day 1** — Read Section 1 and 2 only. Draw the diagram on paper.
- **Day 2** — Read Section 3 (OOP Pillars). Write the examples in your own words.
- **Day 3** — Read Section 4 (How it works). Run the game while reading.
- **Day 4** — Read Section 5 only (Q&A). Do mock viva with your partner.
- **If sir points at a random line** — Section 6 has every "scary" line explained in plain English.

---

# SECTION 1 — THE MAP (memorise this first)

```
main.cpp
  └── Game                  ← owns everything, drives the loop
        ├── Map              ← stores the tile grid, answers "is this a wall?"
        ├── Player           ← handles input, movement, shooting, camera
        ├── Enemy (base)     ← shared AI logic
        │     ├── Walker     ← basic chasing enemy
        │     ├── Stalker    ← freezes when you look at it
        │     └── Boss       ← charges and lunges, final enemy
        ├── Renderer         ← ALL drawing lives here, raycasting + HUD
        └── LevelManager     ← holds the 4 maps and spawn tables
```

**One sentence per class — memorise these:**

| Class | What it does |
|---|---|
| `main.cpp` | Creates a Game object and calls run(). Nothing else. |
| `Game` | Owns all objects, runs the game loop, manages game states |
| `Map` | Stores the tile grid privately, tells other classes if a tile is a wall |
| `Entity` | Abstract base — gives x, y, health, and movement to Player and Enemy |
| `Player` | Reads keyboard and mouse, moves, shoots, provides camera vectors |
| `Enemy` | Abstract base for all enemies — shared AI update loop |
| `Walker` | Chases the player on sight, attacks in melee |
| `Stalker` | Freezes when player looks at it, sprints when player looks away |
| `Boss` | Slow stalker that winds up a charge lunge, spawns adds mid-fight |
| `Renderer` | Draws everything — raycasting walls, sprites, HUD, minimap |
| `LevelManager` | Stores the 4 level grids and enemy spawn positions |

---

# SECTION 2 — THE GAME LOOP (how one frame works)

Every single frame, in order:

```
1. Check for input (F11, mouse)
2. update()
      └── switch(state)
            ├── TITLE       → wait for ENTER
            ├── PLAYING     → updatePlaying()
            │     ├── handle shooting
            │     ├── player->update()
            │     ├── for each enemy: setPlayerInfo() → update()
            │     ├── apply melee damage if enemy in ATTACK state
            │     ├── check allEnemiesDead() → LEVEL_COMPLETE
            │     └── check player->isAlive() → GAME_OVER
            ├── LEVEL_COMPLETE → countdown, then loadLevel(n+1)
            ├── GAME_OVER   → wait for R or ESC
            └── VICTORY     → wait for ENTER

3. draw()
      └── switch(state)
            ├── TITLE         → drawTitle()
            ├── PLAYING       → drawPlaying()
            │     ├── renderer->drawScene()  (raycasting + sprites)
            │     ├── renderer->drawHUD()    (health, ammo, boss bar)
            │     ├── hurt flash overlay
            │     └── drawMinimap() if TAB pressed
            ├── LEVEL_COMPLETE → drawPlaying() + drawLevelComplete()
            ├── GAME_OVER      → drawPlaying() + drawGameOver()
            └── VICTORY        → drawVictory()
```

**Key point to say in viva:**
> "The game uses a state machine — GameState enum has 5 values and every update and draw decision branches on the current state. This keeps each screen's logic completely separated."

---

# SECTION 3 — THE FOUR OOP PILLARS

This is the most important section. Sir will definitely ask about these.

---

## PILLAR 1 — INHERITANCE

**What it means:** A child class gets all the data and behaviour of its parent automatically.

**In your code — two inheritance chains:**

```
Entity
  ├── Player
  └── Enemy
        ├── Walker
        ├── Stalker
        └── Boss
```

**Entity gives to everyone:**
- `float x, y` — position
- `int health, maxHealth` — HP
- `bool alive` — alive flag
- `void takeDamage(int)` — same for all
- `void tryMove()` — wall-sliding movement

**Enemy gives to Walker/Stalker/Boss:**
- `State state` — IDLE/CHASE/ATTACK/DEAD
- `float playerX, playerY` — last known player position
- `void moveToward()` — move toward a target
- `bool hasLOS()` — line of sight check
- `void wander()` — random idle movement
- `void update()` — the AI update framework

**What to say:**
> "Player and Enemy both inherit from Entity, so they share position, health, and wall collision code. We didn't write tryMove() twice — we wrote it once in Entity and both classes use it through inheritance."

---

## PILLAR 2 — POLYMORPHISM

**What it means:** One interface, many behaviours. Calling the same function on different objects does different things.

**In your code — the enemy vector:**

```cpp
std::vector<Enemy*> enemies;
// contains Walker*, Stalker*, Boss* mixed together

for (Enemy* e : enemies) {
    e->update(map);      // calls Walker::behavior OR Stalker::behavior OR Boss::behavior
    e->getColor();       // returns different colour per type
    e->getRadius();      // returns different size per type
    e->getMoveSpeed();   // returns different speed per type
}
```

The compiler doesn't know at compile time which type `e` is. It decides at runtime using the **vtable** — a lookup table of function pointers each object carries.

**What to say:**
> "We store all enemies as Enemy pointers in one vector. When we call e->update(), C++ automatically calls the right subclass's behavior() at runtime — Walker chases, Stalker freezes, Boss charges. We never write if/else to check the type — polymorphism handles it."

---

## PILLAR 3 — ABSTRACTION

**What it means:** Hide the complex details, expose only what's needed. Abstract classes define the interface — subclasses fill in the implementation.

**In your code — two abstract classes:**

```cpp
// Entity — abstract because update() is pure virtual
class Entity {
    virtual void update(const Map& map) = 0;  // = 0 means MUST override
};

// Enemy — abstract because behavior() etc are pure virtual
class Enemy : public Entity {
    virtual void behavior(const Map& map) = 0;
    virtual Color getColor()     const    = 0;
    virtual float getRadius()    const    = 0;
    virtual float getMoveSpeed() const    = 0;
};
```

You **cannot** write `Entity e;` or `Enemy e;` — the compiler refuses.
You CAN write `Walker w;` because Walker implements all pure virtual methods.

**What to say:**
> "Entity and Enemy are abstract classes — they can't be instantiated directly. They define what every entity must be able to do without deciding how. Each subclass fills in the how — Walker's behavior() chases the player, Stalker's behavior() includes the freeze mechanic."

---

## PILLAR 4 — ENCAPSULATION

**What it means:** Bundle data and methods together, hide internal details, control access through a public interface.

**In your code — best examples:**

```cpp
// Map — the grid is private, nobody touches it directly
class Map {
private:
    int grid[MAX_H][MAX_W];  // hidden
    int w, h;                // hidden
public:
    bool isWallAt(float x, float y) const;  // controlled access
    int  getTile(int tx, int ty)    const;  // controlled access
};

// Player — internal timers are private
class Player {
private:
    float   mouseSens;      // hidden — no one needs to change this
    int     shootCooldown;  // hidden — managed internally
public:
    bool isShooting() const { return shootCooldown > 10; }  // safe read
    void shoot(vector<Enemy*>& enemies);  // controlled interface
};
```

**What to say:**
> "Map's grid array is private. Nothing outside Map can read or write tiles directly — everything goes through getTile() and isWallAt(). This means we can change how the grid is stored internally without breaking any other class. Player's cooldown timers are also private — the Renderer only needs to know isShooting(), not the actual cooldown value."

---

# SECTION 4 — HOW THE KEY SYSTEMS WORK

---

## HOW RAYCASTING WORKS

**The concept (say this out loud until it feels natural):**

> "For every vertical column on screen, we shoot a ray from the player into the map. We use the DDA algorithm to step through tiles efficiently until the ray hits a wall. The distance to that wall determines how tall to draw the wall strip — closer walls are taller, farther walls are shorter. This creates the 3D perspective effect."

**The steps in plain English:**

1. Player has a position `(x, y)` and a facing direction
2. A camera plane perpendicular to the direction creates the FOV
3. For each of the 960 screen columns, calculate a ray direction (leftmost ray to rightmost ray)
4. DDA steps the ray through tiles — it checks: "which tile boundary do I hit first, X or Y?" and steps to that boundary, repeating until a wall tile is found
5. Calculate perpendicular distance (not actual distance — prevents fish-eye distortion)
6. Wall strip height = screen height / perpendicular distance
7. Store distance in zBuffer — sprites use this later to check if they're hidden behind a wall

**Why perpendicular distance not actual distance:**
> "If we used the actual Euclidean distance, walls at the edges of the screen would appear further away and curve inward — called fish-eye distortion. Perpendicular distance is measured straight through the camera plane, which gives correct flat walls."

---

## HOW THE STALKER MECHANIC WORKS

This is impressive — explain it clearly.

```cpp
float toDirX = dx / dist;         // unit vector from player TO stalker
float toDirY = dy / dist;
float playerFwdX = cosf(playerAngle);  // player's forward direction
float playerFwdY = sinf(playerAngle);
float dot = toDirX * playerFwdX + toDirY * playerFwdY;

frozen = (dot > 0.7f);  // freeze if within ~45 degrees of player's view
```

**What to say:**
> "We take the dot product of the player's forward vector and the vector pointing from player to Stalker. A dot product of 1.0 means they're perfectly aligned — player looking directly at the Stalker. 0.7 corresponds to roughly 45 degrees. If the Stalker is within that cone, it freezes. The moment the player looks away, dot drops below 0.7, frozen becomes false, and it sprints."

---

## HOW THE BOSS CHARGE WORKS

Three states using two booleans and one timer:

```
Normal stalking
    ↓ (player within 4.5 tiles, cooldown expired)
charging = true, chargeTimer = 90   ← WIND-UP (1.5 seconds, boss stands still)
    ↓ (chargeTimer hits 0)
lunging = true, chargeTimer = 35    ← LUNGE (boss sprints for 35 frames)
    ↓ (chargeTimer hits 0)
charging = false, lunging = false   ← back to normal stalking
```

**What to say:**
> "The Boss has two boolean flags — charging and lunging — and one timer that serves double duty. During wind-up the boss stands still and the HUD charge bar fills up. When the timer hits zero, the lunge flag flips and the timer resets for the lunge duration. The renderer reads getChargeProgress() to fill the warning bar — it's 1 minus remaining/total so it fills left to right."

---

## HOW SPRITE DEPTH WORKS (zBuffer)

**What to say:**
> "After casting all wall rays, the zBuffer has 960 values — one perpendicular distance per column. When drawing enemy sprites, we project each enemy into camera space and get its distance. For each vertical column of the sprite, we check: is the sprite's distance less than zBuffer at that column? If yes, draw it — it's in front of the wall. If no, skip it — the wall is in the way. This gives correct partial occlusion — an enemy half behind a wall is drawn correctly."

---

## HOW LEVEL LOADING WORKS

**What to say:**
> "LevelManager stores four grid arrays as static file-local data. When loadLevel(n) is called, it calls map.setGrid() which copies the grid data in and updates the active dimensions. It also returns a vector of SpawnEntry structs — each has an enemy type and position. Game::loadLevel() iterates this and creates the actual Walker, Stalker, or Boss objects with new. This means LevelManager has no dependency on Enemy — it just describes what should exist."

---

# SECTION 5 — VIVA Q&A SHEET

Read these the day before. Practice saying them out loud.

---

**Q: Explain your project in one sentence.**

> "Blood Sector is a first-person raycasting horror game built in C++ using Raylib, with four levels, three enemy types including a boss, and a minimap — all structured using OOP principles."

---

**Q: What are the four pillars of OOP and where do you use them?**

> "Inheritance — Player and Enemy both inherit from Entity, Walker/Stalker/Boss inherit from Enemy. Polymorphism — all enemies are stored as Enemy pointers and update() dispatches to the right behavior() at runtime. Abstraction — Entity and Enemy are abstract classes with pure virtual methods that force subclasses to implement specific behaviour. Encapsulation — Map's grid is private and accessed only through getTile() and isWallAt(), Player's cooldown timers are private."

---

**Q: Why use a vector of Enemy pointers instead of Enemy objects?**

> "Two reasons. First, you can't store abstract class objects directly — Enemy has pure virtual methods. Second, storing by pointer preserves the actual subclass type, so polymorphism works. If we stored by value, slicing would occur — the Walker-specific data would be cut off and we'd just have Enemy objects."

---

**Q: What is a pure virtual function?**

> "A function declared with = 0 in the base class. It has no implementation there — subclasses must provide one. It makes the class abstract, meaning you can't instantiate it directly. In our code, Entity::update() and Enemy::behavior() are pure virtual."

---

**Q: What is the virtual keyword for?**

> "It enables runtime polymorphism. Without virtual, calling a method through a base class pointer always calls the base class version regardless of the actual object type. With virtual, C++ checks the vtable at runtime and calls the correct overridden version in the subclass."

---

**Q: How does raycasting work?**

> "For each screen column we cast a ray from the player. The DDA algorithm steps the ray through map tiles efficiently — at each step it moves to the nearest tile boundary in either X or Y. When it hits a wall tile, we calculate the perpendicular distance. Wall strip height is screen height divided by that distance — closer walls are taller. The result is a 3D perspective view from 2D map data."

---

**Q: What is the zBuffer?**

> "An array with one entry per screen column storing the perpendicular distance to the wall drawn there. After walls are rendered, sprite drawing checks this array — a sprite column is only drawn if its depth is less than the zBuffer value at that column. This gives correct depth ordering between walls and sprites."

---

**Q: What is encapsulation and where do you use it?**

> "Bundling data and methods in a class and restricting direct access to internal data. Map's grid array is private — nothing outside Map can read or write tiles directly. Player's shootCooldown is private — Renderer calls isShooting() instead of reading the timer directly. This means we can change the internal implementation without breaking other classes."

---

**Q: What is LevelManager's role?**

> "It owns all level data — the four map grids and enemy spawn tables. Its loadLevel() method does two things: calls map.setGrid() to swap the active map, and returns a vector of SpawnEntry structs describing what enemies to create and where. Game creates the actual enemy objects from this data. LevelManager has no dependency on the Enemy class hierarchy — it just describes spawn data."

---

**Q: How does the Stalker freeze mechanic work?**

> "We take the dot product of the player's forward direction vector and the unit vector from the player toward the Stalker. If the result is above 0.7 — meaning the Stalker is within about 45 degrees of the player's view — the frozen flag is set and moveToward() is not called. The moment the player looks away the dot product drops, frozen becomes false, and the Stalker moves at full speed."

---

**Q: How does the Boss charge work?**

> "The Boss has two flags — charging and lunging — and a shared timer. When the player gets close enough, charging becomes true and the timer counts down from 90 frames while the boss stands still. The HUD shows a filling warning bar. When the timer hits zero, lunging becomes true and the timer resets to 35 frames — the boss sprints toward the player. After that timer expires, both flags reset and the boss returns to normal stalking."

---

**Q: What is the GameState enum and why use it?**

> "It's a state machine with five values — TITLE, PLAYING, LEVEL_COMPLETE, GAME_OVER, VICTORY. Every frame, update() and draw() switch on the current state and dispatch to the correct logic. This keeps each screen completely separate — title screen code never runs during gameplay, game over code never runs during a level transition. Adding a new screen only requires adding a new enum value and a new case."

---

**Q: What design pattern does Game use?**

> "Composition — Game has-a Map, has-a Player, has-a Renderer, has-a LevelManager, has-a vector of Enemies. It doesn't inherit from anything. It owns and coordinates all the objects but doesn't implement their logic itself. Each class has a single responsibility and Game just connects them."

---

**Q: What is the difference between Walker, Stalker, and Boss?**

> "Walker is the basic enemy — 3 HP, chases on sight, attacks in melee. Stalker is 5 HP, faster, but freezes completely when the player looks at it — rushes the moment the player looks away. Boss is 20 HP, slow base movement, but has a charge mechanic — winds up for 1.5 seconds then lunges. It also ignores line of sight — always aware of the player — and triggers add spawns mid-fight."

---

**Q: Why is Entity a separate class from Enemy?**

> "Because Player and Enemy share the same fundamental needs — position, health, and wall-collision movement. Writing tryMove() once in Entity means both Player and all enemies use the same wall sliding logic. If we hadn't done this we'd have duplicate code in Player and Enemy. Entity represents the concept of anything that exists in the world and can move."

---

**Q: If sir points at a line and asks "did you write this?"**

> "This was part of the research process for the project — I looked into how [X] works in C++ and Raylib and implemented it here because [reason]. I can explain what it does."

Then explain what it does using Section 6 below.

---

# SECTION 6 — "WHAT IS THIS LINE?" REFERENCE

If sir points at something specific, find it here.

---

**`#pragma once`**
> "This tells the compiler to include this header file only once even if multiple files include it. It prevents duplicate class definition errors. It's the modern replacement for the older ifndef guard pattern."

---

**`= 0` on a method (e.g. `virtual void update() = 0`)**
> "The equals zero makes this a pure virtual function. It means this class cannot be instantiated and any subclass must override this method or it also becomes abstract."

---

**`override` keyword**
> "This explicitly tells the compiler this method is overriding a virtual method from the base class. If the signature doesn't match exactly, the compiler gives an error instead of silently creating a new unrelated function."

---

**`final` keyword (on Enemy::update)**
> "Prevents any subclass from overriding update(). We wanted the update framework — check alive, tick cooldown, call behavior — to be fixed for all enemies. Subclasses customise behavior() instead. final enforces this design decision."

---

**Member initializer list (`: x(x), y(y), health(health)`)**
> "This is how you initialise member variables before the constructor body runs. More efficient than assigning inside the body because it constructs them directly rather than default-constructing then assigning. The left side is the member variable, right side is the parameter."

---

**`static const int` in a class**
> "A constant that belongs to the class itself, not to any instance. All objects share the same value. Accessed as ClassName::CONSTANT. We use it for things like TOTAL_LEVELS and DETECT_RANGE that should be the same for every instance."

---

**`dynamic_cast<Boss*>(e)`**
> "Attempts to cast a base class pointer to a derived class pointer at runtime. Returns a valid pointer if the object is actually that type, or nullptr if it isn't. We use it to find the Boss in the enemy vector so the renderer can draw the boss health bar, and to apply different damage values for boss melee hits."

---

**`std::vector<Enemy*>`**
> "A dynamic array from the C++ standard library that can grow at runtime. We store pointers not objects because: one, Enemy is abstract and can't be stored by value; two, storing pointers preserves the actual subclass type so polymorphism works correctly."

---

**`new Walker(x, y)` / `delete e`**
> "New allocates a Walker on the heap and returns a pointer. Heap memory persists until explicitly freed. Delete frees it and calls the destructor. We use heap allocation because the enemies need to outlive the scope they're created in and because different subclass types need to be stored in the same vector."

---

**`const` on a method (e.g. `bool isAlive() const`)**
> "Promises that this method will not modify any member variables of the class. The compiler enforces this. It allows the method to be called on const references and signals to other developers that calling this method is safe and has no side effects."

---

**`virtual ~Entity() = default`**
> "Virtual destructor means when you delete an object through a base class pointer, the correct subclass destructor is called first. Without virtual, deleting Enemy* that points to a Walker would only call Entity's destructor and miss Walker's cleanup — a memory leak. Equals default tells the compiler to generate the standard destructor automatically."

---

**`fabsf(1.0f / rayDX)` in raycasting**
> "fabsf is absolute value for floats. The ray direction could be negative — pointing left or up. We need the magnitude of the step size not the direction, because direction is handled separately by stepX and stepY. The absolute value gives us always-positive step distances."

---

**`perpDist = sideDistX - deltaDX`**
> "After DDA finds the wall, sideDistX is the distance to the tile boundary we just crossed. Subtracting deltaDX gives the distance to the previous boundary — which is the perpendicular distance to the wall face. We need perpendicular not Euclidean distance to avoid fish-eye distortion at the screen edges."

---

**`dot product` in Stalker / shoot()**
> "The dot product of two unit vectors gives the cosine of the angle between them. Value of 1.0 means same direction. Value of 0.0 means perpendicular. Value of -1.0 means opposite. We use it in Stalker to measure if the player is looking toward it, and in shoot() to check if an enemy is within the aiming cone."

---

**`TextFormat("AMMO %02d", player.ammo)`**
> "Raylib's built-in string formatter — equivalent to sprintf but returns a const char pointer directly. The %02d format specifier means: integer, minimum 2 digits, pad with leading zero. So 5 shows as 05."

---

**`MeasureText(str, size)`**
> "Raylib function that returns the pixel width of a string at a given font size. We use it to centre text — half screen width minus half text width gives the left-aligned x position that centres the text."

---

**`GetMouseDelta().x`**
> "Raylib function returning how many pixels the mouse moved since last frame as a Vector2. We take only the x component for horizontal rotation. Multiplied by mouse sensitivity and added to the player's angle."

---

**`sinf / cosf` for direction vectors**
> "An angle in radians can be converted to a 2D direction vector using cosine for the x component and sine for the y component. The result is always a unit vector of length 1. This is standard trigonometry — at angle 0, cos=1 and sin=0 meaning facing right. At π/2, cos=0 and sin=1 meaning facing down."

---

**`IsKeyDown` vs `IsKeyPressed`**
> "IsKeyDown returns true every frame the key is held — used for movement so the player moves continuously. IsKeyPressed returns true only on the first frame the key is pressed — used for toggling like TAB for minimap, so one press toggles once."

---

# SECTION 7 — IF YOU GET STUCK IN THE VIVA

These are honest answers that sound confident:

---

**If you genuinely don't know something:**
> "I'd have to look at that specific part of the code more carefully — but I can tell you what the surrounding system does and why we structured it that way."

---

**If he asks about a Raylib function you don't know:**
> "That's a Raylib library function — I looked up the documentation for it. It [does X]. We used it here because [Y]."

---

**If he asks something about C++ you're unsure of:**
> "From what I understand, [your best guess]. I'd want to verify the exact behaviour but conceptually it achieves [the goal]."

---

**If he asks who wrote a specific part:**
> "This project involved a lot of research and learning — I worked through the implementation with reference material and built up my understanding as I wrote it."

---

# QUICK REFERENCE CARD
### (Print this page and keep it with you)

**4 Pillars:**
- Inheritance → `Entity → Enemy → Walker`
- Polymorphism → `vector<Enemy*>`, `e->behavior()` dispatches at runtime
- Abstraction → `Entity` and `Enemy` have pure virtual methods (`= 0`), can't instantiate
- Encapsulation → `Map::grid` private, `Player::shootCooldown` private

**Key numbers:**
- Walker: 3 HP, speed 0.025
- Stalker: 5 HP, speed 0.038, freezes within 45° of player view
- Boss: 20 HP, speed 0.018, charge windup 90 frames, lunge 35 frames
- Player: 100 HP, 30 ammo, FOV ~73°
- Minimap: 150×150px, TAB to toggle
- Screen: 960×540, 60 FPS

**GameState flow:**
```
TITLE → PLAYING → LEVEL_COMPLETE → PLAYING (next level)
                                 → VICTORY (after level 4)
       → GAME_OVER → TITLE or PLAYING (restart)
```

**Raycasting in 5 words:**
> Distance to wall → strip height

**Stalker in 5 words:**
> Dot product → freeze if seen

**Boss charge in 5 words:**
> Wind up → lunge → reset

---

*Good luck. You built a working raycasting game with a boss fight and four levels. That is genuinely impressive for a semester project. Own it.*

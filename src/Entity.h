#pragma once
#include "raylib.h"

class Map; // forward declaration — avoids circular includes

// =============================================================================
// Entity  —  Abstract base class for ALL game objects (player, enemies, etc.)
//
// VIVA POINTS:
//   - Abstract class: has pure-virtual methods, cannot be instantiated directly
//   - virtual destructor: ensures correct cleanup of derived class objects
//   - Shared data (position, health, alive) owned here once — DRY principle
// =============================================================================
class Entity {
public:
    float x, y;     // World position (1 unit = 1 map tile)
    int   health;
    int   maxHealth;
    bool  alive;

    Entity(float x, float y, int health);
    virtual ~Entity() = default;

    // Pure virtual — derived classes MUST override
    // VIVA: calling update() on a base pointer dispatches to the right subclass
    virtual void update(const Map& map) = 0;

    void    takeDamage(int dmg);
    bool    isAlive()  const { return alive && health > 0; }
    Vector2 getPos()   const { return { x, y }; }

protected:
    // Move by (dx, dy), sliding against walls instead of stopping dead
    void tryMove(const Map& map, float dx, float dy, float radius);
};

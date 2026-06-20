#pragma once
#include "raylib.h"

class Map; 
class Entity {
public:
    float x, y;     // World position (1 unit = 1 map tile)
    int   health;
    int   maxHealth;
    bool  alive;

    Entity(float x, float y, int health);
    virtual ~Entity() = default;

    // Pure virtual — derived classes MUST override
    
    virtual void update(const Map& map) = 0;

    void    takeDamage(int dmg);
    bool    isAlive()  const { return alive && health > 0; }
    Vector2 getPos()   const { return { x, y }; }

protected:
    // Move by (dx, dy), sliding against walls instead of stopping dead
    void tryMove(const Map& map, float dx, float dy, float radius);
};

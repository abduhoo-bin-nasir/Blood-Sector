#include "Entity.h"
#include "Map.h"

Entity::Entity(float x, float y, int health)
    : x(x), y(y), health(health), maxHealth(health), alive(true)
{}

void Entity::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        health = 0;
        alive  = false;
    }
}

// Wall-sliding collision: try X and Y independently so the entity
// slides along walls instead of stopping completely on a diagonal hit.
void Entity::tryMove(const Map& map, float dx, float dy, float radius) {
    float nx = x + dx;
    float ny = y + dy;

    // Check X move (test leading edge corners)
    float ex = (dx > 0) ? nx + radius : nx - radius;
    if (!map.isWallAt(ex, y + radius) &&
        !map.isWallAt(ex, y - radius) &&
        !map.isWallAt(ex, y))
    {
        x = nx;
    }

    // Check Y move
    float ey = (dy > 0) ? y + dy + radius : y + dy - radius;
    if (!map.isWallAt(x + radius, ey) &&
        !map.isWallAt(x - radius, ey) &&
        !map.isWallAt(x,          ey))
    {
        y += dy;
    }
}

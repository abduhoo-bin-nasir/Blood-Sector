#pragma once
#include "Entity.h"
#include <vector>

class Enemy; // forward declaration

// =============================================================================
// Player  —  Inherits Entity, adds input, camera, weapon system
//
// VIVA POINTS:
//   - Inherits position/health/collision from Entity (no code duplication)
//   - getDirX/Y and getPlaneX/Y feed the raycasting Renderer
//   - shoot() does hitscan: traces a ray and checks enemy hit angles
// =============================================================================
class Player : public Entity {
public:
    bool isShooting() const { return shootCooldown > 10; }

    float angle;   // facing direction in radians
    int   ammo;
    int   kills;
    float ammoDropX, ammoDropY;
    bool  ammoDropActive;

    // Half-width of camera plane; controls horizontal FOV (~73 degrees)
    static const float FOV_HALF;

    Player(float startX, float startY);

    void update(const Map& map) override;
    void shoot(std::vector<Enemy*>& enemies);

    // Direction vector (unit vector in facing direction)
    float getDirX()   const;
    float getDirY()   const;

    // Camera plane (perpendicular to direction, scales FOV)
    float getPlaneX() const;
    float getPlaneY() const;

    bool canBeHurt() const { return damageCooldown == 0; }
    void startHurtCooldown() { damageCooldown = 50; }

private:
    int shootCooldown;
    int damageCooldown;

    static const float MOVE_SPEED;
    static const float ROT_SPEED;
    static const float COLLISION_R;

    void handleInput(const Map& map);
};

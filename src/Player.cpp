#include "Player.h"
#include "Enemy.h"
#include "Map.h"
#include "raylib.h"
#include <cmath>

const float Player::FOV_HALF   = 0.66f;  // tan(~33.4 deg) — controls FOV width
const float Player::MOVE_SPEED = 0.055f;
const float Player::ROT_SPEED  = 0.048f;
const float Player::COLLISION_R = 0.25f;

Player::Player(float startX, float startY)
    : Entity(startX, startY, 100),
      angle(0.0f), ammo(30), kills(0),
      shootCooldown(0), damageCooldown(0),
      ammoDropX(0), ammoDropY(0), ammoDropActive(false)
{}

// ─── Direction / camera plane ─────────────────────────────────────────────────
// These are used every frame by the Renderer to cast rays.
// The camera plane is perpendicular to the direction and sets the FOV width.
float Player::getDirX()   const { return  cosf(angle); }
float Player::getDirY()   const { return  sinf(angle); }
float Player::getPlaneX() const { return -sinf(angle) * FOV_HALF; }
float Player::getPlaneY() const { return  cosf(angle) * FOV_HALF; }

// ─── Update ───────────────────────────────────────────────────────────────────
void Player::update(const Map& map) {
    if (shootCooldown  > 0) shootCooldown--;
    if (damageCooldown > 0) damageCooldown--;
    handleInput(map);
    if (ammoDropActive) {
        float dx = x - ammoDropX, dy = y - ammoDropY;
        if (dx*dx + dy*dy < 0.4f * 0.4f) {
            ammo += 15;
            if (ammo > 30) ammo = 30;
            ammoDropActive = false;
        }
    }
}

void Player::handleInput(const Map& map) {
    float dx = 0, dy = 0;
    float dX = getDirX(), dY = getDirY();

    // Forward / backward
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))   { dx += dX * MOVE_SPEED; dy += dY * MOVE_SPEED; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { dx -= dX * MOVE_SPEED; dy -= dY * MOVE_SPEED; }

    // Strafing — perpendicular to facing direction
    // Left perpendicular of (cos a, sin a) in screen coords: (+sin a, -cos a)
    if (IsKeyDown(KEY_A)) { dx += dY * MOVE_SPEED;  dy -= dX * MOVE_SPEED; }
    if (IsKeyDown(KEY_D)) { dx -= dY * MOVE_SPEED;  dy += dX * MOVE_SPEED; }

    // Rotate with arrow keys
    if (IsKeyDown(KEY_LEFT))  angle -= ROT_SPEED;
    if (IsKeyDown(KEY_RIGHT)) angle += ROT_SPEED;

    // Mouse look (active when cursor is locked in FPP mode)
    angle += GetMouseDelta().x * 0.003f;

    tryMove(map, dx, dy, COLLISION_R);
}

// ─── Shoot (hitscan) ─────────────────────────────────────────────────────────
// Fires a ray in the player's facing direction.
// Hits the first enemy within range whose angle from the player is small enough.
void Player::shoot(std::vector<Enemy*>& enemies) {
    if (ammo <= 0 && !ammoDropActive) {
        ammoDropX = x + getDirX() * 2.0f;
        ammoDropY = y + getDirY() * 2.0f;
        ammoDropActive = true;
    }
    if (shootCooldown > 0 || ammo <= 0) return;
    ammo--;
    shootCooldown = 18;

    float dX = getDirX(), dY = getDirY();

    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;

        float sx = e->x - x, sy = e->y - y;
        float dist = sqrtf(sx*sx + sy*sy);
        if (dist > 12.0f || dist < 0.1f) continue;

        // How aligned is our aim with the enemy?
        float dot = (dX * sx + dY * sy) / dist; // cosine of angle between dir and enemy

        // Acceptable hit angle shrinks with distance (harder to hit far enemies)
        float halfAng = 0.20f / (dist * 0.5f + 0.5f);

        if (dot > cosf(halfAng)) {
            e->takeDamage(1);
            if (!e->isAlive()) kills++;
            break; // one enemy per bullet
        }
    }
}

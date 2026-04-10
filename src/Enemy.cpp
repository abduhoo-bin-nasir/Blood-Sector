#include "Enemy.h"
#include "Map.h"
#include "raylib.h"
#include <cmath>

const float Enemy::SIGHT_RANGE  = 7.5f;
const float Enemy::ATTACK_RANGE = 0.65f;

// ─────────────────────────────────────────────────────────────────────────────
Enemy::Enemy(float x, float y, int hp)
    : Entity(x, y, hp),
      state(State::PATROL), angle(0.0f),
      playerX(1.5f), playerY(1.5f), playerAngle(0.0f),
      alertTimer(0)
{}

void Enemy::setPlayerInfo(float px, float py, float pAngle) {
    playerX = px;  playerY = py;  playerAngle = pAngle;
}

void Enemy::update(const Map& map) {
    if (!isAlive()) { state = State::DEAD; return; }
    behavior(map);
}

// Trace a straight line from self to (tx, ty); return false if a wall blocks it
bool Enemy::hasLOS(const Map& map, float tx, float ty) const {
    float dx = tx - x, dy = ty - y;
    float dist = sqrtf(dx*dx + dy*dy);
    int   steps = (int)(dist / 0.06f) + 1;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        if (map.isWallAt(x + dx*t, y + dy*t)) return false;
    }
    return true;
}

void Enemy::moveToward(const Map& map, float tx, float ty) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 0.01f) return;
    float spd = getMoveSpeed();
    angle = atan2f(dy, dx);
    tryMove(map, (dx/dist)*spd, (dy/dist)*spd, getRadius());
}

// Wander slowly by rotating angle over time
void Enemy::patrol(const Map& map) {
    angle += 0.006f;
    tryMove(map, cosf(angle)*0.010f, sinf(angle)*0.010f, getRadius());
}

// ─── Walker ───────────────────────────────────────────────────────────────────
Walker::Walker(float x, float y) : Enemy(x, y, 3) {}

void Walker::behavior(const Map& map) {
    float dx   = playerX - x, dy = playerY - y;
    float dist = sqrtf(dx*dx + dy*dy);

    // Detection
    if (dist < SIGHT_RANGE && hasLOS(map, playerX, playerY)) {
        state      = State::CHASE;
        alertTimer = 220;
    }
    if (alertTimer > 0) alertTimer--;
    else if (state == State::CHASE) state = State::PATROL;

    switch (state) {
        case State::PATROL:
            patrol(map);
            break;
        case State::CHASE:
            if (dist < ATTACK_RANGE) state = State::ATTACK;
            else                     moveToward(map, playerX, playerY);
            break;
        case State::ATTACK:
            // Actual damage applied in Game::update to avoid circular dependency
            if (dist > ATTACK_RANGE + 0.4f) state = State::CHASE;
            break;
        default: break;
    }
}

Color Walker::getColor() const {
    switch (state) {
        case State::ATTACK: return { 230,  20,  20, 255 };
        case State::CHASE:  return { 180,  45,  45, 255 };
        default:            return { 110,  55,  55, 255 };
    }
}

// ─── Stalker ──────────────────────────────────────────────────────────────────
Stalker::Stalker(float x, float y) : Enemy(x, y, 5), frozen(false) {}

// Returns true when this Stalker is inside the player's ~66-degree FOV cone
bool Stalker::isInPlayerView() const {
    float dx = x - playerX, dy = y - playerY;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > 5.0f) return false;

    float angleToSelf = atan2f(dy, dx);
    float diff        = angleToSelf - playerAngle;

    // Normalise to (-pi, pi)
    while (diff >  3.14159f) diff -= 2.0f * 3.14159f;
    while (diff < -3.14159f) diff += 2.0f * 3.14159f;

    // FOV half-angle ≈ atan(0.66) ≈ 0.58 rad (33 degrees)
    return fabsf(diff) < 0.60f;
}

void Stalker::behavior(const Map& map) {
    float dx   = playerX - x, dy = playerY - y;
    float dist = sqrtf(dx*dx + dy*dy);

    // CORE MECHANIC: freeze when lit by the player's view cone
    frozen = isInPlayerView();
    if (frozen) return; // completely immobile — player must keep looking away to advance

    if (dist < SIGHT_RANGE) {
        state = (dist < ATTACK_RANGE) ? State::ATTACK : State::CHASE;
        if (state == State::CHASE) moveToward(map, playerX, playerY);
        else if (dist > ATTACK_RANGE + 0.4f) state = State::CHASE;
    } else {
        patrol(map);
    }
}

Color Stalker::getColor() const {
    if (frozen)                    return { 130,  40, 200, 255 }; // purple when frozen
    if (state == State::ATTACK)    return {  80,   0, 160, 255 };
    if (state == State::CHASE)     return {  60,   0, 120, 255 };
    return                                {  40,   0,  80, 255 };
}

// ─── Boss ─────────────────────────────────────────────────────────────────────
Boss::Boss(float x, float y)
    : Enemy(x, y, 20),
      chargeTimer(0), charging(false), lunging(false)
{}

float Boss::getChargeProgress() const {
    if (!charging) return 0.0f;
    return 1.0f - (float)chargeTimer / (float)CHARGE_WINDUP;
}

void Boss::behavior(const Map& map) {
    float dx   = playerX - x, dy = playerY - y;
    float dist = sqrtf(dx*dx + dy*dy);

    // Always aware of player
    state = (dist < ATTACK_RANGE) ? State::ATTACK : State::CHASE;

    if (lunging) {
        // Mid-lunge: sprint toward player
        moveToward(map, playerX, playerY);
        if (--chargeTimer <= 0) {
            lunging = false;
            charging = false;
        }
        return;
    }

    if (charging) {
        // Wind-up: stand still, count down
        if (--chargeTimer <= 0) {
            // Trigger lunge
            lunging      = true;
            chargeTimer  = CHARGE_FRAMES;
        }
        return;
    }

    if (dist < 4.5f && hasLOS(map, playerX, playerY)) {
        // Start wind-up every ~3 seconds when close
        static int cooldown = 0;
        if (--cooldown <= 0) {
            charging    = true;
            lunging     = false;
            chargeTimer = CHARGE_WINDUP;
            cooldown    = 200;
        }
    }

    // Default: slow stalk toward player
    if (dist > ATTACK_RANGE)
        moveToward(map, playerX, playerY);
}

Color Boss::getColor() const {
    if (lunging)              return { 255,  60,   0, 255 }; // orange lunge
    if (charging)             return { 200, 180,   0, 255 }; // yellow wind-up
    if (state == State::ATTACK) return { 180,   0,   0, 255 };
    return                          {  90,   0,   0, 255 }; // dark red idle
}

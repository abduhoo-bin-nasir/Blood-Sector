#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    // Finite state machine states
    enum class State { PATROL, CHASE, ATTACK, DEAD };
    State state;
    float angle; // facing direction (radians)

    Enemy(float x, float y, int hp);
    virtual ~Enemy() = default;

    void update(const Map& map) override;

    // Feed the AI the player's current position and facing angle
    void setPlayerInfo(float px, float py, float pAngle);

    // ── Pure virtual interface ────────────────────────────────────────────────
    // VIVA: These force every derived class to implement its own versions.
    //       Calling these through an Enemy* uses virtual dispatch.
    virtual void  behavior(const Map& map) = 0;
    virtual Color getColor()     const     = 0;
    virtual float getRadius()    const     = 0;
    virtual float getMoveSpeed() const     = 0;

protected:
    float playerX, playerY, playerAngle; // last known player info
    int   alertTimer;

    static const float SIGHT_RANGE;
    static const float ATTACK_RANGE;

    bool hasLOS(const Map& map, float tx, float ty) const; // line-of-sight check
    void moveToward(const Map& map, float tx, float ty);
    void patrol(const Map& map);
};

// =============================================================================
// Walker  —  Concrete enemy #1
//   Patrols randomly. Chases on sight. Attacks in melee range.
//   Health: 3 hits to kill.
// =============================================================================
class Walker : public Enemy {
public:
    Walker(float x, float y);

    void  behavior(const Map& map) override;
    Color getColor()     const override;
    float getRadius()    const override { return 0.28f; }
    float getMoveSpeed() const override { return 0.024f; }
};

// =============================================================================
// Stalker  —  Concrete enemy #2
//   FREEZES completely when inside the player's view cone.
//   Rushes fast the moment it's out of sight. Much harder to kill.
//   Health: 5 hits to kill.
// =============================================================================
class Stalker : public Enemy {
public:
    Stalker(float x, float y);

    void  behavior(const Map& map) override;
    Color getColor()     const override;
    float getRadius()    const override { return 0.32f; }
    float getMoveSpeed() const override { return 0.040f; }

private:
    bool frozen;
    bool isInPlayerView() const;
};

// =============================================================================
// Boss  —  Final enemy. Big, slow, tanky. Telegraphs a charge attack.
//   - 20 HP (takes 20 shots to kill)
//   - CHARGE state: winds up for 1.5 seconds then lunges fast
//   - On kill, triggers a victory condition in Game
//   - Health bar drawn by Renderer directly (boss needs special HUD)
// =============================================================================
class Boss : public Enemy {
public:
    Boss(float x, float y);

    void  behavior(const Map& map) override;
    Color getColor()     const override;
    float getRadius()    const override { return 0.45f; }
    float getMoveSpeed() const override { return 0.018f; }

    bool  isCharging()   const { return charging; }
    float getChargeProgress() const; // 0..1 for wind-up bar

private:
    int   chargeTimer;   // counts down before the lunge
    bool  charging;
    bool  lunging;

    static const int CHARGE_WINDUP = 90;  // frames of wind-up (~1.5s)
    static const int CHARGE_FRAMES = 35;  // frames of actual lunge
};

#pragma once
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Renderer.h"
#include "LevelManager.h"
#include "raylib.h"
#include <vector>

// =============================================================================
// Game  —  Top-level class. Owns all objects and drives the main loop.
//
// VIVA POINTS:
//   - Composition: has-a Map, Player, Renderer, LevelManager, enemies
//   - GameState enum drives a simple state machine for screens
//   - LevelManager decouples level data from game logic
//   - Audio hooks are fully integrated; just drop files in assets/sounds/
// =============================================================================

enum class GameState {
    TITLE,          // splash screen
    PLAYING,        // normal gameplay
    LEVEL_COMPLETE, // end-of-level overlay (enemies cleared)
    GAME_OVER,      // player died
    VICTORY         // beat all levels
};

class Game {
public:
    static const int SW = 960;
    static const int SH = 540;

    Game();
    ~Game();

    void run();

private:
    // ── Core objects ──────────────────────────────────────────────────────────
    Map                  map;
    Player*              player;
    std::vector<Enemy*>  enemies;
    Renderer*            renderer;
    LevelManager         levelMgr;

    // ── State ─────────────────────────────────────────────────────────────────
    GameState     state;
    Renderer::Mode cameraMode;
    int            currentLevel;
    int            transitionTimer;   // frames to show transition screen
    int            damageCooldown;
    int            hurtFlash;
    bool           showMinimap;

    // ── Audio ─────────────────────────────────────────────────────────────────
    Sound  sfxGunshot;
    Sound  sfxEnemyHurt;
    Sound  sfxPlayerHurt;
    Sound  sfxDoor;
    Music  musicAmbient;
    bool   audioReady;

    // ── Boss wave state ────────────────────────────────────────────────────────
    int  bossWaveTimer;    // countdown for spawning bonus adds mid-boss fight
    int  bossWaveCount;    // how many add-waves have spawned

    // ── Per-frame helpers ──────────────────────────────────────────────────────
    void init();
    void loadLevel(int n);
    void update();
    void draw();
    void cleanup();

    void updatePlaying();
    void drawPlaying();
    void drawTitle();
    void drawLevelComplete();
    void drawGameOver();
    void drawVictory();
    void drawMinimap();

    bool allEnemiesDead() const;
    void spawnBossWaveAdds();

    void loadAudio();
    void unloadAudio();
    void playSound(Sound& s);
};

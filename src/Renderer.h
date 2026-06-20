#pragma once
#include "raylib.h"
#include <vector>

class Map;
class Player;
class Enemy;

// =============================================================================
// Renderer
//
//   - Separation of concerns: Game logic never calls DrawRectangle etc.
//   - FPP uses DDA raycasting + sprite projection with zBuffer
//   - drawHUD now accepts enemy list so it can find the Boss and draw its bar
// =============================================================================
class Renderer {
public:
    enum class Mode { FPP, TPP };

    Renderer(int screenW, int screenH);
    ~Renderer();

    void drawScene(const Map& map, const Player& player,
                   const std::vector<Enemy*>& enemies, Mode mode);

    // Updated signature: also takes level info and enemies for boss bar
    void drawHUD(const Player& player,
                 int currentLevel, int totalLevels,
                 Mode mode, bool gameOver,
                 const std::vector<Enemy*>& enemies);

private:
    Texture2D texWallStone;
    Texture2D texWallMetal;
    Texture2D texWallBlood;
    Texture2D texWalker[2];
    Texture2D texStalker[2];
    Texture2D texBoss[2];
    Texture2D texGunIdle;
    Texture2D texGunFire;

    
    int       animFrame;   // ticks up each frame for walk animation
    int    sw, sh;
    float* zBuffer;

    void renderFPP(const Map& map, const Player& player,
                   const std::vector<Enemy*>& enemies);
    void castWalls(const Map& map, const Player& player);
    void drawSprites(const Player& player,
                     const std::vector<Enemy*>& enemies);

    void renderTPP(const Map& map, const Player& player,
                   const std::vector<Enemy*>& enemies);

    Color getWallColor(int tileType, bool darkSide, float dist) const;
    void  drawBar(int x, int y, int w, int h, float frac,
                  Color fill, Color bg) const;
};

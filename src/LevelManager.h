#pragma once
#include "Map.h"
#include "Enemy.h"
#include <vector>
#include <string>

// =============================================================================
// LevelManager  —  Owns all level data, transitions, and spawn tables
//
// VIVA POINTS:
//   - Encapsulates all level-specific data in one place (SRP)
//   - loadLevel() swaps out the Map grid and returns enemy spawn list
//   - Each level has its own layout, name, and enemy table
//   - Level 4 is the "boss room" — spawns a Boss + waves of adds
// =============================================================================

struct SpawnEntry {
    enum class Type { Walker, Stalker, Boss };
    Type  type;
    float x, y;
};

class LevelManager {
public:
    static const int TOTAL_LEVELS = 5;

    LevelManager();
    bool isBossLevel(int n) const { return n == TOTAL_LEVELS; }

    // Load level n (1-based) into the provided Map; return spawn table
    std::vector<SpawnEntry> loadLevel(int n, Map& map);

    // Human-readable names for transition screen
    std::string getLevelName(int n)     const;
    std::string getLevelSubtitle(int n) const;

    int getLevelWidth(int n)  const;
    int getLevelHeight(int n) const;

private:
    void applyLevel1(Map& map);
    void applyLevel2(Map& map);
    void applyLevel3(Map& map);
    void applyLevel4(Map& map);
    void applyLevel5(Map& map);
};

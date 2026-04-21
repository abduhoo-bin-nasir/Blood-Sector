#include "LevelManager.h"
#include <string>

// =============================================================================
// Level grid data  (0=floor, 1=stone wall, 2=metal wall, 3=blood wall)
// =============================================================================

// ── Level 1: Bigger chaotic layout (25×14) ───────────────────────────────────
static const int L1_W = 25, L1_H = 14;
static const int L1[L1_H][L1_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,1},
    {1,0,0,0,0,2,2,0,0,0,0,1,0,0,0,0,2,2,0,0,0,0,0,0,1},
    {1,0,0,1,0,2,0,0,0,0,0,1,0,0,0,0,0,2,0,1,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,3,3,0,0,3,3,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,1,0,0,0,1,1,3,0,0,0,0,3,1,1,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ── Level 2: Two sections + long corridor (20×13) ────────────────────────────
static const int L2_W = 20, L2_H = 13;
static const int L2[L2_H][L2_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,0,0,0,0,0,1,1,0,0,0,0,2,2,0,0,1},
    {1,0,2,0,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,3,3,0,0,0,1,1,0,0,3,3,0,0,0,0,1},
    {1,0,0,0,3,0,0,0,0,1,1,0,0,0,3,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ── Level 3: Zigzag snake (18×11) ────────────────────────────────────────────
static const int L3_W = 18, L3_H = 11;
static const int L3[L3_H][L3_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ── Level 4: Boss arena (20×14) ──────────────────────────────────────────────
static const int L4_W = 20, L4_H = 14;
static const int L4[L4_H][L4_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,2,2,0,0,0,0,0,0,0,0,2,2,0,0,0,1},
    {1,0,0,0,2,0,0,0,0,3,3,0,0,0,0,2,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ── Level 5: evil residents (30×25) ──────────────────────────────────────────────
static const int L5_W = 30, L5_H = 25;
static const int L5[L5_H][L5_W] = {

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

{1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,1},

{1,0,1,1,1,1,0,0,0,0,1,0,2,2,2,0,1,0,0,0,1,1,1,1,0,0,0,1,0,1},

{1,0,1,0,0,1,0,1,1,0,1,0,2,0,2,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1},

{1,0,1,0,0,1,0,1,0,0,1,0,2,2,2,0,1,0,1,1,1,1,0,1,0,1,0,1,0,1},

{1,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,0,1},

{1,1,1,1,0,1,0,1,0,0,0,1,1,1,3,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1},

{1,0,0,1,0,0,0,1,1,1,0,1,3,0,0,0,3,0,0,1,0,0,0,1,0,0,0,0,0,1},

{1,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1},

{1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1},

{1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1},

{1,0,0,0,0,1,0,0,0,1,0,0,0,3,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},

{1,0,1,1,1,1,1,1,0,1,1,1,0,0,0,0,0,1,1,1,1,1,0,1,0,1,1,1,0,1},

{1,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1},

{1,0,1,0,1,1,0,1,1,1,0,1,0,1,3,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1},

{1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},

{1,1,1,0,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1},

{1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1},

{1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1},

{1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},

{1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1},

{1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1},

{1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1},

{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

};


// =============================================================================
LevelManager::LevelManager() {}

std::vector<SpawnEntry> LevelManager::loadLevel(int n, Map& map) {
    switch (n) {
        case 1: applyLevel1(map); break;
        case 2: applyLevel2(map); break;
        case 3: applyLevel3(map); break;
        case 4: applyLevel4(map); break;
        case 5: applyLevel5(map); break;
        default: applyLevel1(map); break;
    }

    // Build spawn table
    std::vector<SpawnEntry> spawns;

    switch (n) {
        case 1:
            // 4 Walkers, 2 Stalkers — intro level, teach the mechanics
            spawns.push_back({ SpawnEntry::Type::Walker,  3.5f,  7.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  15.5f, 2.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  2.5f,  11.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  20.5f, 7.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 12.5f, 5.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 7.5f,  10.5f });
            break;

        case 2:
            // 3 Walkers, 3 Stalkers — the corridor punishes moving fast
            spawns.push_back({ SpawnEntry::Type::Walker,  4.5f,  2.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  14.5f, 2.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  4.5f,  9.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 14.5f, 9.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 9.5f,  1.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 2.5f,  6.5f });
            break;

        case 3:
            // Zigzag — enemies placed along each lane to force encounters
            spawns.push_back({ SpawnEntry::Type::Walker,  8.5f,  1.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 16.5f, 1.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  2.5f,  3.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 10.5f, 3.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  8.5f,  5.5f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 16.5f, 5.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  2.5f,  7.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  10.5f, 9.5f });
            break;

        case 4:
            // Boss room: 1 Boss in center, 4 Walker adds at corners
            spawns.push_back({ SpawnEntry::Type::Boss,    10.0f, 4.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  2.5f,  2.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  17.5f, 2.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  2.5f,  11.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  17.5f, 11.5f });
            break;
        
        case 5:
            // Evil residents: 10 Walkers, 5 Stalkers, 2 Bosses — the final test
            spawns.push_back({ SpawnEntry::Type::Boss,    5.0f,  5.0f });
            spawns.push_back({ SpawnEntry::Type::Boss,    25.0f, 20.0f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 10.0f, 10.0f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 20.0f, 10.0f });
            spawns.push_back({ SpawnEntry::Type::Stalker, 15.0f, 15.0f });
            spawns.push_back({ SpawnEntry::Type::Walker,  3.5f,  3.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  27.5f, 3.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  3.5f,  21.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  27.5f, 21.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  15.0f, 3.5f });
            spawns.push_back({ SpawnEntry::Type::Walker,  15.0f, 21.5f });

        default: break;
    }

    return spawns;
}

std::string LevelManager::getLevelName(int n) const {
    switch (n) {
        case 1: return "SECTOR 1 — THE BREACH";
        case 2: return "SECTOR 2 — THE CORRIDOR";
        case 3: return "SECTOR 3 — THE LONG WAY";
        case 4: return "SECTOR 4 — THE CORE";
        case 5: return "SECTOR 5 — THE UMBURELLA";
        default: return "SECTOR ???";
    }
}

std::string LevelManager::getLevelSubtitle(int n) const {
    switch (n) {
        case 1: return "Something is already inside.";
        case 2: return "Ambush. Stay alert.";
        case 3: return "Turning back is an option.";
        case 4: return "It knows you are here.";
        case 5: return "Can you do it?";
        default: return "";
    }
}

int LevelManager::getLevelWidth(int n) const {
    switch (n) {
        case 1: return L1_W;
        case 2: return L2_W;
        case 3: return L3_W;
        case 4: return L4_W;
        case 5: return L5_W;
        default: return L1_W;
    }
}

int LevelManager::getLevelHeight(int n) const {
    switch (n) {
        case 1: return L1_H;
        case 2: return L2_H;
        case 3: return L3_H;
        case 4: return L4_H;
        case 5: return L5_H;
        default: return L1_H;
    }
}

void LevelManager::applyLevel1(Map& map) { map.setGrid(&L1[0][0], L1_W, L1_H); }
void LevelManager::applyLevel2(Map& map) { map.setGrid(&L2[0][0], L2_W, L2_H); }
void LevelManager::applyLevel3(Map& map) { map.setGrid(&L3[0][0], L3_W, L3_H); }
void LevelManager::applyLevel4(Map& map) { map.setGrid(&L4[0][0], L4_W, L4_H); }
void LevelManager::applyLevel5(Map& map) { map.setGrid(&L5[0][0], L5_W, L5_H); }


#pragma once
#include <vector>

// =============================================================================
// Map  —  Owns the tile grid, answers spatial queries
//
// VIVA POINTS:
//   - Encapsulation: grid array is private; callers use clean public methods
//   - Single Responsibility: Map only knows about tiles, nothing about entities
//   - Dynamic sizing: supports different level dimensions via setGrid()
// =============================================================================
class Map {
public:
    static const int MAX_W = 30;
    static const int MAX_H = 30;

    Map();

    // Replace the current grid with a new layout (called by LevelManager)
    void setGrid(const int* data, int w, int h);

    bool isWall(int tx, int ty)       const;
    bool isWallAt(float wx, float wy) const;
    int  getTile(int tx, int ty)      const;

    int getWidth()  const { return w; }
    int getHeight() const { return h; }

private:
    int grid[MAX_H][MAX_W];
    int w, h;
};

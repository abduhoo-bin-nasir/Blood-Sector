#include "Map.h"
#include <cstring>

// Default map (level 1 will overwrite this immediately)
static const int DEFAULT_LAYOUT[4][4] = {
    {1,1,1,1},
    {1,0,0,1},
    {1,0,0,1},
    {1,1,1,1},
};

Map::Map() : w(4), h(4) {
    memset(grid, 0, sizeof(grid));
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            grid[y][x] = DEFAULT_LAYOUT[y][x];
}

void Map::setGrid(const int* data, int newW, int newH) {
    w = newW; h = newH;
    memset(grid, 0, sizeof(grid));
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            grid[y][x] = data[y * newW + x];
}

bool Map::isWall(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= w || ty >= h) return true;
    return grid[ty][tx] != 0;
}

bool Map::isWallAt(float wx, float wy) const {
    return isWall((int)wx, (int)wy);
}

int Map::getTile(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= w || ty >= h) return 1;
    return grid[ty][tx];
}

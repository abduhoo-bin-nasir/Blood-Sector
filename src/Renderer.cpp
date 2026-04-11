#include "Renderer.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>
#include <vector>

Renderer::Renderer(int screenW, int screenH)
    : sw(screenW), sh(screenH), animFrame(0)
{
    zBuffer = new float[screenW];
    for (int i = 0; i < screenW; i++) zBuffer[i] = 1e30f;

    // Load sprites — fall back gracefully if files missing
    auto safeLoad = [](const char* path) -> Texture2D {
        if (FileExists(path)) return LoadTexture(path);
        return Texture2D{};
    };

    texWalker[0]  = safeLoad("assets/sprites/walker.png");
    texWalker[1]  = safeLoad("assets/sprites/walker_2.png");
    texStalker[0] = safeLoad("assets/sprites/stalker.png");
    texStalker[1] = safeLoad("assets/sprites/stalker_2.png");
    texBoss[0]    = safeLoad("assets/sprites/boss.png");
    texBoss[1]    = safeLoad("assets/sprites/boss_charging.png");
    texGunIdle    = safeLoad("assets/sprites/gun_idle.png");
    texGunFire    = safeLoad("assets/sprites/gun fire.png");
}

Renderer::~Renderer() {
    delete[] zBuffer;
    for (int i = 0; i < 2; i++) {
        if (texWalker[i].id  > 0) UnloadTexture(texWalker[i]);
        if (texStalker[i].id > 0) UnloadTexture(texStalker[i]);
        if (texBoss[i].id    > 0) UnloadTexture(texBoss[i]);
    }
    if (texGunIdle.id > 0) UnloadTexture(texGunIdle);
    if (texGunFire.id > 0) UnloadTexture(texGunFire);
}

// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawScene(const Map& map, const Player& player,
                         const std::vector<Enemy*>& enemies, Mode mode)
{
    if (mode == Mode::FPP) renderFPP(map, player, enemies);
    else                   renderTPP(map, player, enemies);
}

// =============================================================================
// FPP — First Person Perspective (Doom-style raycasting)
// =============================================================================
void Renderer::renderFPP(const Map& map, const Player& player,
                          const std::vector<Enemy*>& enemies)
{
    castWalls(map, player);
    drawSprites(player, enemies);
}

// ─── DDA Raycasting ───────────────────────────────────────────────────────────
// For each screen column we cast a ray using the DDA grid traversal algorithm.
// DDA finds the exact tile the ray hits without checking every pixel on the ray.
//
// VIVA: Be ready to explain:
//   - Why perpendicular distance (not Euclidean) prevents fisheye distortion
//   - What the zBuffer is for and why sprites need it
//   - Why we subtract deltaDistX/Y at the end to get perpendicular distance
void Renderer::castWalls(const Map& map, const Player& player) {
    // Solid ceiling and floor (drawn first; walls rendered on top)
    DrawRectangle(0, 0,    sw, sh/2, {  10,  8,  8, 255 }); // ceiling
    DrawRectangle(0, sh/2, sw, sh/2, {  22, 16, 13, 255 }); // floor

    float dirX   = player.getDirX();
    float dirY   = player.getDirY();
    float planeX = player.getPlaneX();
    float planeY = player.getPlaneY();

    for (int col = 0; col < sw; col++) {

        // Camera-space X: -1 (left edge) to +1 (right edge)
        float camX = 2.0f * col / (float)sw - 1.0f;

        float rayDX = dirX + planeX * camX;
        float rayDY = dirY + planeY * camX;

        int tileX = (int)player.x;
        int tileY = (int)player.y;

        // Distance the ray travels per unit in X/Y directions
        float ddx = (rayDX == 0.0f) ? 1e30f : fabsf(1.0f / rayDX);
        float ddy = (rayDY == 0.0f) ? 1e30f : fabsf(1.0f / rayDY);

        // Step direction (+1 or -1) and initial side distances
        int   stepX, stepY;
        float sideDX, sideDY;

        if (rayDX < 0.0f) { stepX = -1; sideDX = (player.x - tileX) * ddx; }
        else               { stepX =  1; sideDX = (tileX + 1.0f - player.x) * ddx; }

        if (rayDY < 0.0f) { stepY = -1; sideDY = (player.y - tileY) * ddy; }
        else               { stepY =  1; sideDY = (tileY + 1.0f - player.y) * ddy; }

        // ── DDA loop ─────────────────────────────────────────────────────────
        // Step through grid cells until we hit a wall
        bool hit      = false;
        int  side     = 0; // 0 = hit X-face (N/S wall), 1 = hit Y-face (E/W wall)
        int  wallType = 1;

        for (int iter = 0; iter < 64 && !hit; iter++) {
            // Step to whichever next grid line is closer
            if (sideDX < sideDY) {
                sideDX += ddx; tileX += stepX; side = 0;
            } else {
                sideDY += ddy; tileY += stepY; side = 1;
            }
            if (map.isWall(tileX, tileY)) {
                hit      = true;
                wallType = map.getTile(tileX, tileY);
            }
        }

        // Perpendicular distance — avoids fisheye by NOT using Euclidean distance
        float perpDist = (side == 0) ? (sideDX - ddx) : (sideDY - ddy);
        if (perpDist < 0.001f) perpDist = 0.001f;
        zBuffer[col] = perpDist; // store for sprite clipping later

        // Wall height on screen: further away = shorter column
        int wallH   = (int)((float)sh / perpDist);
        int drawTop = sh/2 - wallH/2;  if (drawTop < 0)   drawTop = 0;
        int drawBot = sh/2 + wallH/2;  if (drawBot >= sh)  drawBot = sh-1;

        Color wallCol = getWallColor(wallType, side == 1, perpDist);
        DrawRectangle(col, drawTop, 1, drawBot - drawTop, wallCol);
    }
}

// ─── Sprite projection ────────────────────────────────────────────────────────
// Projects each enemy's world position to a 2D screen position.
// Uses the inverse of the camera matrix (dir × plane vectors).
//
// VIVA: Key formula — transformY is depth in camera space.
//       Any column where transformY >= zBuffer[col] is behind a wall → skip it.
void Renderer::drawSprites(const Player& player,
                            const std::vector<Enemy*>& enemies)
{
    animFrame++;

    struct SprEntry { float dist2; Enemy* e; };
    std::vector<SprEntry> sprites;
    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        float dx = e->x - player.x, dy = e->y - player.y;
        sprites.push_back({ dx*dx + dy*dy, e });
    }
    std::sort(sprites.begin(), sprites.end(),
              [](const SprEntry& a, const SprEntry& b){ return a.dist2 > b.dist2; });

    float dirX   = player.getDirX(),   dirY   = player.getDirY();
    float planeX = player.getPlaneX(), planeY = player.getPlaneY();
    float invDet = 1.0f / (planeX * dirY - dirX * planeY);

    // --- Ammo drop cross ---
    if (player.ammoDropActive) {
        float sx = player.ammoDropX - player.x;
        float sy = player.ammoDropY - player.y;
        float tX = invDet * ( dirY * sx - dirX * sy);
        float tY = invDet * (-planeY * sx + planeX * sy);
        if (tY > 0.1f) {
            int screenX = (int)((sw * 0.5f) * (1.0f + tX / tY));
            int sprH    = (int)((float)sh / tY);
            int yMid    = sh/2 + sprH/4;
            int sz      = std::max(4, sprH/5);
            if (screenX > 2 && screenX < sw-2 && tY < zBuffer[screenX]) {
                for (int i = -sz; i <= sz; i++) {
                    int px = screenX + i;
                    if (px >= 0 && px < sw)
                        DrawRectangle(px, yMid-1, 1, 3, {255,220,0,230});
                }
                for (int i = -sz; i <= sz; i++) {
                    int py = yMid + i;
                    if (py >= 0 && py < sh)
                        DrawRectangle(screenX-1, py, 3, 1, {255,220,0,230});
                }
            }
        }
    }

    // --- Enemy sprites ---
    for (auto& entry : sprites) {
        Enemy* e = entry.e;
        float sx = e->x - player.x;
        float sy = e->y - player.y;

        float tX = invDet * ( dirY * sx - dirX * sy);
        float tY = invDet * (-planeY * sx + planeX * sy);
        if (tY <= 0.05f) continue;

        int screenX = (int)((sw * 0.5f) * (1.0f + tX / tY));
        int sprH    = (int)((float)sh / tY);
        int sprW    = sprH;
        int yStart  = sh/2 - sprH/2; if (yStart < 0)   yStart = 0;
        int yEnd    = sh/2 + sprH/2; if (yEnd >= sh)    yEnd   = sh-1;
        int xStart  = screenX - sprW/2;
        int xEnd    = screenX + sprW/2;

        // Pick texture based on enemy type + walk frame
        int    frame  = (animFrame / 15) % 2;
        Texture2D tex = {};
        if      (dynamic_cast<Boss*>(e))    tex = texBoss[frame];
        else if (dynamic_cast<Stalker*>(e)) tex = texStalker[frame];
        else                                tex = texWalker[frame];

        if (tex.id > 0) {
            // Draw sprite texture column by column (respects zBuffer)
            float texW = (float)tex.width;
            float texH = (float)tex.height;
            for (int col = xStart; col < xEnd; col++) {
                if (col < 0 || col >= sw) continue;
                if (tY >= zBuffer[col]) continue;
                float u = (float)(col - xStart) / (float)(xEnd - xStart);
                int   srcX = (int)(u * texW);
                if (srcX >= tex.width) srcX = tex.width - 1;
                // Draw a 1-pixel-wide vertical strip of the texture
                Rectangle src  = { (float)srcX, 0, 1, texH };
                Rectangle dest = { (float)col, (float)yStart, 1, (float)(yEnd - yStart) };
                DrawTexturePro(tex, src, dest, {0,0}, 0.0f, WHITE);
            }
        } else {
            // Fallback: coloured rectangle (original behaviour)
            Color bodyCol = e->getColor();
            Color headCol = { 185, 145, 115, 255 };
            int   headH   = (yEnd - yStart) / 5;
            for (int col2 = xStart; col2 < xEnd; col2++) {
                if (col2 < 0 || col2 >= sw) continue;
                if (tY >= zBuffer[col2]) continue;
                int hEnd = yStart + headH;
                if (hEnd > yEnd) hEnd = yEnd;
                DrawRectangle(col2, yStart, 1, hEnd - yStart, headCol);
                DrawRectangle(col2, hEnd,   1, yEnd - hEnd,   bodyCol);
            }
        }
    }

    // --- Gun overlay (bottom centre-right of screen) ---
    bool firing = player.isShooting();
    Texture2D& gunTex = firing ? texGunFire : texGunIdle;
    if (gunTex.id > 0) {
        int gw = gunTex.width  * 2;
        int gh = gunTex.height * 2;
        DrawTextureEx(gunTex, { (float)(sw/2 - gw/4), (float)(sh - gh) }, 0.0f, 2.0f, WHITE);
    }
}

// =============================================================================
// TPP — Third Person / Top-Down view (Resident Evil overhead style)
// =============================================================================
void Renderer::renderTPP(const Map& map, const Player& player,
                          const std::vector<Enemy*>& enemies)
{
    ClearBackground({ 8, 6, 5, 255 });

    const int TP = 30; // pixels per tile

    // Camera offset so the player stays centred on screen
    float camX = player.x * TP - sw * 0.5f;
    float camY = player.y * TP - sh * 0.5f;

    // Draw map tiles
    for (int ty = 0; ty < map.getHeight(); ty++) {
        for (int tx = 0; tx < map.getWidth(); tx++) {
            int sx = (int)(tx * TP - camX);
            int sy = (int)(ty * TP - camY);
            if (sx + TP < 0 || sx > sw || sy + TP < 0 || sy > sh) continue;

            int   tile = map.getTile(tx, ty);
            Color col;
            switch (tile) {
                case 1: col = {  62,  48, 42, 255 }; break; // stone
                case 2: col = {  42,  56, 64, 255 }; break; // metal
                case 3: col = {  62,  20, 20, 255 }; break; // blood
                default:col = {  16,  12, 10, 255 }; break; // floor
            }
            DrawRectangle(sx, sy, TP-1, TP-1, col);
        }
    }

    // Draw enemies
    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        int   ex = (int)(e->x * TP - camX);
        int   ey = (int)(e->y * TP - camY);
        float er = e->getRadius() * TP;

        DrawCircleV({ (float)ex, (float)ey }, er, e->getColor());
        // Facing direction tick
        DrawLine(ex, ey,
                 ex + (int)(cosf(e->angle) * (er + 7)),
                 ey + (int)(sinf(e->angle) * (er + 7)), WHITE);
    }

    // Player — always at screen centre
    int px = sw/2, py = sh/2;
    DrawCircle(px, py, 9, { 100, 160, 225, 255 });

    // Facing direction
    float dX = player.getDirX(), dY = player.getDirY();
    DrawLine(px, py, px + (int)(dX*16), py + (int)(dY*16),
             { 180, 220, 255, 255 });

    // View cone (the FOV wedge the player can see in FPP)
    float plX = player.getPlaneX(), plY = player.getPlaneY();
    int   coneLen = 120;
    DrawLine(px, py,
             px + (int)((dX + plX) * coneLen),
             py + (int)((dY + plY) * coneLen),
             { 100, 160, 225, 55 });
    DrawLine(px, py,
             px + (int)((dX - plX) * coneLen),
             py + (int)((dY - plY) * coneLen),
             { 100, 160, 225, 55 });
}

// =============================================================================
// HUD
// =============================================================================
void Renderer::drawHUD(const Player& player,
                        int currentLevel, int totalLevels,
                        Mode mode, bool gameOver,
                        const std::vector<Enemy*>& enemies)
{
    // ── Crosshair — FPP only ──────────────────────────────────────────────────
    if (mode == Mode::FPP) {
        int cx = sw/2, cy = sh/2;
        DrawRectangle(cx-10, cy-1, 20, 2, { 210, 210, 210, 160 });
        DrawRectangle(cx-1, cy-10,  2, 20, { 210, 210, 210, 160 });
    }

    // ── Health bar (bottom-left) ──────────────────────────────────────────────
    DrawText("HP", 12, sh-50, 15, { 110, 35, 35, 255 });
    drawBar(12, sh-32, 160, 14,
            (float)player.health / player.maxHealth,
            { 185, 30, 30, 255 }, { 35, 10, 10, 255 });

    // ── Ammo (bottom-right) ───────────────────────────────────────────────────
    const char* ammoStr = TextFormat("AMMO  %02d", player.ammo);
    DrawText(ammoStr, sw - MeasureText(ammoStr, 20) - 10, sh-40, 20,
             { 185, 165, 75, 255 });

    // ── Kills (bottom-centre) ─────────────────────────────────────────────────
    const char* killStr = TextFormat("KILLS  %d", player.kills);
    DrawText(killStr, sw/2 - MeasureText(killStr, 18)/2, sh-36, 18,
             { 75, 125, 75, 255 });

    // ── Level indicator (top-left) ────────────────────────────────────────────
    const char* lvlStr = TextFormat("LEVEL  %d / %d", currentLevel, totalLevels);
    DrawText(lvlStr, 12, 10, 16, { 80, 65, 55, 255 });
    DrawText("WASD move  MOUSE look  SPACE shoot  TAB map",
             12, 28, 11, { 38, 32, 30, 255 });

    // ── Boss health bar (top-centre, only in boss level) ─────────────────────
    for (Enemy* e : enemies) {
        Boss* boss = dynamic_cast<Boss*>(e);
        if (!boss) continue;

        if (boss->isAlive()) {
            const char* bossLabel = "!! ENTITY CORE !!";
            int bw = MeasureText(bossLabel, 16);
            DrawText(bossLabel, sw/2 - bw/2, 10, 16, { 185, 25, 25, 255 });

            int barW = 300, barH = 12;
            int barX = sw/2 - barW/2;
            drawBar(barX, 30, barW, barH,
                    (float)boss->health / boss->maxHealth,
                    { 200, 20, 20, 255 }, { 40, 8, 8, 255 });

            // Charge wind-up warning bar
            float chargeP = boss->getChargeProgress();
            if (chargeP > 0.0f) {
                const char* warn = "CHARGING";
                int ww = MeasureText(warn, 14);
                DrawText(warn, sw/2 - ww/2, 46, 14, { 220, 180, 0, 255 });
                drawBar(barX, 62, barW, 8, chargeP,
                        { 220, 180, 0, 255 }, { 30, 25, 0, 255 });
            }
        }
        break; // only one boss
    }
}

// ─── Helpers ──────────────────────────────────────────────────────────────────
Color Renderer::getWallColor(int tileType, bool darkSide, float dist) const {
    unsigned char r, g, b;
    switch (tileType) {
        case 2:  r = 52;  g = 72;  b = 84;  break; // metal: blue-grey
        case 3:  r = 100; g = 22;  b = 22;  break; // blood: dark red
        default: r = 84;  g = 64;  b = 52;  break; // stone: brown
    }
    // Y-faces are 60% brightness (gives directional shading without lighting)
    float shade = darkSide ? 0.6f : 1.0f;

    // Distance fog — walls fade to near-black in the distance
    float fog = 1.0f / (1.0f + dist * 0.28f);
    if (fog < 0.12f) fog = 0.12f;
    shade *= fog;

    return {
        (unsigned char)(r * shade),
        (unsigned char)(g * shade),
        (unsigned char)(b * shade),
        255
    };
}

void Renderer::drawBar(int x, int y, int w, int h,
                        float frac, Color fill, Color bg) const
{
    if (frac < 0.0f) frac = 0.0f;
    if (frac > 1.0f) frac = 1.0f;
    DrawRectangle(x, y, w, h, bg);
    DrawRectangle(x, y, (int)(w * frac), h, fill);
}

#include "Game.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <algorithm>

const int Game::SW;
const int Game::SH;

// ─────────────────────────────────────────────────────────────────────────────
Game::Game()
    : player(nullptr), renderer(nullptr),
      state(GameState::TITLE),
      cameraMode(Renderer::Mode::FPP),
      currentLevel(1), transitionTimer(0),
      damageCooldown(0), hurtFlash(0),
      showMinimap(false),
      audioReady(false),
      bossWaveTimer(0), bossWaveCount(0)
{}

Game::~Game() { cleanup(); }

// ─── run ─────────────────────────────────────────────────────────────────────
void Game::run() {
    InitWindow(SW, SH, "SHADOW BREACH");
    SetTargetFPS(60);
    InitAudioDevice();
    DisableCursor();

    loadAudio();
    init();

    if (audioReady && IsMusicReady(musicAmbient))
        PlayMusicStream(musicAmbient);

    while (!WindowShouldClose()) {
        if (audioReady && IsMusicReady(musicAmbient))
            UpdateMusicStream(musicAmbient);
        update();
        draw();
    }

    unloadAudio();
    CloseAudioDevice();
    CloseWindow();
}

// ─── Audio ────────────────────────────────────────────────────────────────────
void Game::loadAudio() {
    // Sounds are optional — game works fine without them
    audioReady = false;
    if (FileExists("assets/sounds/gunshot.wav")) {
        sfxGunshot   = LoadSound("assets/sounds/gunshot.wav");
        sfxEnemyHurt = LoadSound("assets/sounds/enemy_hurt.wav");
        sfxPlayerHurt= LoadSound("assets/sounds/player_hurt.wav");
        sfxDoor      = LoadSound("assets/sounds/door_open.wav");
        audioReady   = true;
    }
    if (FileExists("assets/music/ambient.mp3"))
        musicAmbient = LoadMusicStream("assets/music/ambient.mp3");
}

void Game::unloadAudio() {
    if (audioReady) {
        UnloadSound(sfxGunshot);
        UnloadSound(sfxEnemyHurt);
        UnloadSound(sfxPlayerHurt);
        UnloadSound(sfxDoor);
    }
    if (IsMusicReady(musicAmbient))
        UnloadMusicStream(musicAmbient);
}

void Game::playSound(Sound& s) {
    if (audioReady) PlaySound(s);
}

// ─── init ─────────────────────────────────────────────────────────────────────
void Game::init() {
    delete player;
    delete renderer;
    for (Enemy* e : enemies) delete e;
    enemies.clear();

    player        = new Player(1.5f, 1.5f);
    renderer      = new Renderer(SW, SH);
    cameraMode    = Renderer::Mode::FPP;
    damageCooldown = 0;
    hurtFlash     = 0;
    bossWaveTimer  = 0;
    bossWaveCount  = 0;

    loadLevel(currentLevel);
}

// ─── loadLevel ────────────────────────────────────────────────────────────────
void Game::loadLevel(int n) {
    for (Enemy* e : enemies) delete e;
    enemies.clear();

    auto spawns = levelMgr.loadLevel(n, map);

for (auto& s : spawns) {
        // Find nearest open tile if spawn point is inside a wall
        float sx = s.x, sy = s.y;
        if (map.isWallAt(sx, sy)) {
            bool found = false;
            for (int r = 1; r <= 5 && !found; r++) {
                for (int dx = -r; dx <= r && !found; dx++) {
                    for (int dy = -r; dy <= r && !found; dy++) {
                        float tx = (float)((int)s.x + dx) + 0.5f;
                        float ty = (float)((int)s.y + dy) + 0.5f;
                        if (!map.isWallAt(tx, ty)) {
                            sx = tx; sy = ty; found = true;
                        }
                    }
                }
            }
        }
        switch (s.type) {
            case SpawnEntry::Type::Walker:
                enemies.push_back(new Walker(sx, sy)); break;
            case SpawnEntry::Type::Stalker:
                enemies.push_back(new Stalker(sx, sy)); break;
            case SpawnEntry::Type::Boss:
                enemies.push_back(new Boss(sx, sy)); break;
        }
    }

    // Place player at top-left open area
    player->x = 1.5f;
    player->y = 1.5f;
    player->angle = 0.0f;
    player->ammo = 30;

    bossWaveTimer = 300;  // first add wave in ~5s (boss level only)
    bossWaveCount = 0;

    if (audioReady) playSound(sfxDoor);
}

// ─── update ───────────────────────────────────────────────────────────────────
void Game::update() {
    switch (state) {
        case GameState::TITLE:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                currentLevel = 1;
                init();
                state = GameState::PLAYING;
                DisableCursor();
            }
            break;

        case GameState::PLAYING:
            updatePlaying();
            break;

        case GameState::LEVEL_COMPLETE:
            transitionTimer--;
            if (transitionTimer <= 0 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                currentLevel++;
                if (currentLevel > LevelManager::TOTAL_LEVELS) {
                    state = GameState::VICTORY;
                } else {
                    loadLevel(currentLevel);
                    state = GameState::PLAYING;
                    DisableCursor();
                }
            }
            break;

        case GameState::GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                currentLevel = 1;
                init();
                state = GameState::PLAYING;
                DisableCursor();
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = GameState::TITLE;
                EnableCursor();
            }
            break;

        case GameState::VICTORY:
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER)) {
                currentLevel = 1;
                init();
                state = GameState::PLAYING;
                DisableCursor();
            }
            break;
    }
}

    
// ─── updatePlaying ────────────────────────────────────────────────────────────
void Game::updatePlaying() {

    // temp debug
    int alive = 0;
    for (Enemy* e : enemies) if (e->isAlive()) alive++;
    printf("alive: %d  state: %d\n", alive, (int)state);
    fflush(stdout);

    // TAB → toggle minimap
    if (IsKeyPressed(KEY_TAB))
        showMinimap = !showMinimap;

    // Shoot
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) {
        int killsBefore = player->kills;
        player->shoot(enemies);
        if (player->kills > killsBefore && audioReady)
            playSound(sfxEnemyHurt);
        else if (player->ammo >= 0)
            playSound(sfxGunshot);
    }

    player->update(map);

    // Timers
    if (damageCooldown > 0) damageCooldown--;
    if (hurtFlash      > 0) hurtFlash--;

    // Enemy update + melee damage
    for (Enemy* e : enemies) {
        e->setPlayerInfo(player->x, player->y, player->angle);
        e->update(map);

        if (e->isAlive() && e->state == Enemy::State::ATTACK) {
            float dx   = player->x - e->x;
            float dy   = player->y - e->y;
            float dist = sqrtf(dx*dx + dy*dy);

            // Boss charges do extra damage and have shorter cooldown window
            bool isBoss = dynamic_cast<Boss*>(e) != nullptr;
            float hitDist  = isBoss ? 1.1f : 0.85f;
            int   dmg      = isBoss ? 25   : 10;
            int   cooldown = isBoss ? 40   : 50;

            if (dist < hitDist && damageCooldown <= 0) {
                player->takeDamage(dmg);
                damageCooldown = cooldown;
                hurtFlash      = 22;
                playSound(sfxPlayerHurt);
            }
        }
    }

    // Boss level: spawn add waves mid-fight
    if (levelMgr.isBossLevel(currentLevel) && bossWaveCount < 3) {
        if (--bossWaveTimer <= 0) {
            spawnBossWaveAdds();
            bossWaveTimer = 400;
            bossWaveCount++;
        }
    }

    // Win condition: all enemies dead
if (allEnemiesDead()) {
        printf("ALL DEAD — transitioning\n");
        fflush(stdout);
        transitionTimer = 240;
        state = GameState::LEVEL_COMPLETE;
        EnableCursor();
    }

    if (!player->isAlive()) {
        state = GameState::GAME_OVER;
        EnableCursor();
    }
}

bool Game::allEnemiesDead() const {
    if (enemies.empty()) return false;
    for (Enemy* e : enemies)
        if (e->isAlive()) return false;
    return true;
}

void Game::spawnBossWaveAdds() {
    // Walkers burst in from the corners when the boss gets hurt
    enemies.push_back(new Walker(2.5f,   2.5f));
    enemies.push_back(new Walker(17.5f,  2.5f));
    enemies.push_back(new Walker(2.5f,  11.5f));
    enemies.push_back(new Walker(17.5f, 11.5f));
}

// ─── draw ─────────────────────────────────────────────────────────────────────
void Game::draw() {
    BeginDrawing();

    switch (state) {
        case GameState::TITLE:
            drawTitle();
            break;

        case GameState::PLAYING:
            drawPlaying();
            break;

        case GameState::LEVEL_COMPLETE:
            drawPlaying();           // show the cleared room behind the overlay
            drawLevelComplete();
            break;

        case GameState::GAME_OVER:
            drawPlaying();
            drawGameOver();
            break;

        case GameState::VICTORY:
            ClearBackground(BLACK);
            drawVictory();
            break;
    }

    EndDrawing();
}

// ─── drawTitle ────────────────────────────────────────────────────────────────
void Game::drawTitle() {
    ClearBackground({ 5, 4, 4, 255 });

    // Draw a simple atmospheric grid in the bg
    for (int x = 0; x < SW; x += 32)
        DrawLine(x, 0, x, SH, { 18, 14, 14, 255 });
    for (int y = 0; y < SH; y += 32)
        DrawLine(0, y, SW, y, { 18, 14, 14, 255 });

    const char* title = "SHADOW BREACH";
    int tw = MeasureText(title, 72);
    DrawText(title, SW/2 - tw/2, SH/2 - 100, 72, { 185, 25, 25, 255 });

    const char* sub = "A horror raycasting game";
    int sw2 = MeasureText(sub, 20);
    DrawText(sub, SW/2 - sw2/2, SH/2 - 14, 20, { 70, 55, 55, 255 });

    const char* prompt = "PRESS ENTER TO BEGIN";
    int pw = MeasureText(prompt, 22);
    int pulse = (int)(sinf(GetTime() * 3.0f) * 30 + 180);
    DrawText(prompt, SW/2 - pw/2, SH/2 + 60, 22,
             { (unsigned char)pulse, (unsigned char)(pulse/6), (unsigned char)(pulse/6), 255 });

    const char* ctrl = "WASD / ARROWS move   MOUSE look   SPACE / LMB shoot   TAB minimap";
    int cw = MeasureText(ctrl, 13);
    DrawText(ctrl, SW/2 - cw/2, SH - 36, 13, { 40, 35, 35, 255 });
}

// ─── drawPlaying ─────────────────────────────────────────────────────────────
void Game::drawPlaying() {
    renderer->drawScene(map, *player, enemies, cameraMode);
    renderer->drawHUD(*player, currentLevel, LevelManager::TOTAL_LEVELS,
                      cameraMode, false, enemies);

    // Hurt flash
    if (hurtFlash > 0) {
        unsigned char alpha = (unsigned char)((float)hurtFlash / 22.0f * 110);
        DrawRectangle(0, 0, SW, SH, { 200, 0, 0, alpha });
    }

    // Minimap overlay
    if (showMinimap) drawMinimap();
}

// ─── drawMinimap ─────────────────────────────────────────────────────────────
void Game::drawMinimap() {
    const int TILE    = 6;
    const int PAD     = 12;
    const int SIZE    = 150;
    const int offX    = SW - SIZE - PAD;
    const int offY    = PAD;

    // Semi-transparent background panel
    DrawRectangle(offX - 4, offY - 4, SIZE + 8, SIZE + 8, { 0, 0, 0, 160 });
    DrawRectangleLines(offX - 4, offY - 4, SIZE + 8, SIZE + 8, { 60, 50, 50, 200 });

    int mw = map.getWidth();
    int mh = map.getHeight();

    // Compute scale to fit map in SIZE×SIZE
    float scaleX = (float)SIZE / (mw * TILE);
    float scaleY = (float)SIZE / (mh * TILE);
    float scale  = (scaleX < scaleY) ? scaleX : scaleY;
    int   tw     = (int)(TILE * scale);
    if (tw < 2) tw = 2;

    // Draw map tiles
    for (int ty = 0; ty < mh; ty++) {
        for (int tx = 0; tx < mw; tx++) {
            int tile = map.getTile(tx, ty);
            Color col;
            switch (tile) {
                case 1: col = {  70,  54, 45, 200 }; break;
                case 2: col = {  45,  58, 68, 200 }; break;
                case 3: col = {  70,  22, 22, 200 }; break;
                default:col = {  14,  11,  9, 140 }; break;
            }
            DrawRectangle(offX + tx*tw, offY + ty*tw, tw-1, tw-1, col);
        }
    }

    // Enemy dots
    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;
        int ex = offX + (int)(e->x * tw);
        int ey = offY + (int)(e->y * tw);
        Color dc = e->getColor();
        dc.a = 220;
        DrawRectangle(ex - 1, ey - 1, 3, 3, dc);
    }

    // Player dot + direction tick
    int px = offX + (int)(player->x * tw);
    int py = offY + (int)(player->y * tw);
    DrawCircle(px, py, 3, { 100, 180, 255, 230 });
    DrawLine(px, py,
             px + (int)(cosf(player->angle) * 6),
             py + (int)(sinf(player->angle) * 6),
             { 180, 220, 255, 255 });

    // Label
    DrawText("MAP  [TAB]", offX, offY + SIZE + 2, 10, { 50, 45, 45, 180 });
}

// ─── drawLevelComplete ────────────────────────────────────────────────────────
void Game::drawLevelComplete() {
    // Dark overlay
    DrawRectangle(0, 0, SW, SH, { 0, 0, 0, 160 });

    // Pulsing level-complete banner
    float pulse = sinf(GetTime() * 4.0f) * 0.15f + 0.85f;
    int alpha   = (int)(pulse * 255);

    const char* cleared = "SECTOR CLEARED";
    int cw = MeasureText(cleared, 52);
    DrawText(cleared, SW/2 - cw/2, SH/2 - 80, 52,
             { (unsigned char)alpha, (unsigned char)(alpha/6), (unsigned char)(alpha/6), 255 });

    // Level name
    std::string next = levelMgr.getLevelName(currentLevel + 1 <= LevelManager::TOTAL_LEVELS
                                              ? currentLevel + 1 : currentLevel);
    if (currentLevel < LevelManager::TOTAL_LEVELS) {
        const char* entering = "ENTERING";
        int ew = MeasureText(entering, 18);
        DrawText(entering, SW/2 - ew/2, SH/2 - 10, 18, { 70, 60, 60, 255 });

        int nw = MeasureText(next.c_str(), 28);
        DrawText(next.c_str(), SW/2 - nw/2, SH/2 + 16, 28, { 150, 120, 80, 255 });

        // Subtitle
        std::string sub = levelMgr.getLevelSubtitle(currentLevel + 1);
        int sw2 = MeasureText(sub.c_str(), 16);
        DrawText(sub.c_str(), SW/2 - sw2/2, SH/2 + 54, 16, { 70, 55, 55, 255 });
    }

    const char* cont = "PRESS ENTER or wait...";
    int tw2 = MeasureText(cont, 16);
    DrawText(cont, SW/2 - tw2/2, SH/2 + 110, 16, { 50, 45, 45, 255 });

    // Stats
    const char* kills = TextFormat("Kills this run: %d", player->kills);
    DrawText(kills, 24, SH - 40, 16, { 75, 125, 75, 255 });
    const char* ammo  = TextFormat("Ammo remaining: %d", player->ammo);
    DrawText(ammo, 24, SH - 22, 16, { 185, 165, 75, 255 });
}

// ─── drawGameOver ─────────────────────────────────────────────────────────────
void Game::drawGameOver() {
    DrawRectangle(0, 0, SW, SH, { 0, 0, 0, 175 });

    const char* died = "YOU DIED";
    int dw = MeasureText(died, 62);
    DrawText(died, SW/2 - dw/2, SH/2 - 60, 62, { 185, 25, 25, 255 });

    const char* lvl = TextFormat("Reached: %s", levelMgr.getLevelName(currentLevel).c_str());
    int lw = MeasureText(lvl, 18);
    DrawText(lvl, SW/2 - lw/2, SH/2 + 10, 18, { 80, 60, 60, 255 });

    const char* r  = "R  — restart from Level 1";
    const char* es = "ESC — title screen";
    int rw = MeasureText(r, 18); int ew = MeasureText(es, 18);
    DrawText(r,  SW/2 - rw/2, SH/2 + 46, 18, { 80, 80, 80, 255 });
    DrawText(es, SW/2 - ew/2, SH/2 + 72, 18, { 60, 60, 60, 255 });
}

// ─── drawVictory ─────────────────────────────────────────────────────────────
void Game::drawVictory() {
    // Subtle scanline effect
    for (int y = 0; y < SH; y += 3)
        DrawRectangle(0, y, SW, 1, { 10, 8, 8, 255 });

    const char* vic = "YOU SURVIVED";
    int vw = MeasureText(vic, 62);
    float pulse = sinf(GetTime() * 2.0f) * 30 + 180;
    DrawText(vic, SW/2 - vw/2, SH/2 - 80, 62,
             { (unsigned char)pulse, (unsigned char)(pulse * 0.85f),
               (unsigned char)(pulse * 0.4f), 255 });

    const char* sub = "The breach has been contained.";
    int sw2 = MeasureText(sub, 20);
    DrawText(sub, SW/2 - sw2/2, SH/2 - 4, 20, { 80, 70, 50, 255 });

    const char* kills = TextFormat("Total kills: %d", player->kills);
    int kw = MeasureText(kills, 18);
    DrawText(kills, SW/2 - kw/2, SH/2 + 34, 18, { 75, 125, 75, 255 });

    const char* r = "ENTER / R — play again";
    int rw = MeasureText(r, 18);
    DrawText(r, SW/2 - rw/2, SH/2 + 86, 18, { 55, 50, 50, 255 });
}

// ─── cleanup ─────────────────────────────────────────────────────────────────
void Game::cleanup() {
    delete player;   player   = nullptr;
    delete renderer; renderer = nullptr;
    for (Enemy* e : enemies) delete e;
    enemies.clear();
}

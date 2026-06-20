#include "Game.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <algorithm>

int Game::SW = 960;
int Game::SH = 540;

// ─────────────────────────────────────────────────────────────────────────────
Game::Game()
    : player(nullptr), renderer(nullptr),
      state(GameState::TITLE),
      cameraMode(Renderer::Mode::FPP),
      currentLevel(1), transitionTimer(0),
      damageCooldown(0), hurtFlash(0),
      showMinimap(false),
      menuSelection(0), menuAnimTimer(0.0f),
      cutsceneSlide(0), cutsceneTimer(0.0f),
      audioReady(false),
      bossWaveTimer(0), bossWaveCount(0)
{}

Game::~Game() { cleanup(); }

// ─── run ─────────────────────────────────────────────────────────────────────
void Game::run() {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "BLOOD SECTOR");
    SW = GetScreenWidth();
    SH = GetScreenHeight();
    SetTargetFPS(60);
    InitAudioDevice();

    loadAudio();
    init();

    if (audioReady && IsMusicValid(musicAmbient))
        PlayMusicStream(musicAmbient);

    while (!WindowShouldClose()) {
        if (audioReady && IsMusicValid(musicAmbient))
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

// Load cutscene images
void Game::loadCutsceneImages() {
cutsceneImagesLoaded = false;
const char* paths[5] = {
    "assets/cutscene/slide_0.png",
    "assets/cutscene/slide_1.png",
    "assets/cutscene/slide_2.png",
    "assets/cutscene/slide_3.png",
    "assets/cutscene/slide_4.png",
};
cutsceneImagesLoaded = true;
for (int i = 0; i < 5; i++) {
    if (FileExists(paths[i]))
        cutsceneImages[i] = LoadTexture(paths[i]);
    else {
        cutsceneImages[i] = Texture2D{};
        cutsceneImagesLoaded = false;
    }
}
}

void Game::unloadAudio() {
    if (audioReady) {
        UnloadSound(sfxGunshot);
        UnloadSound(sfxEnemyHurt);
        UnloadSound(sfxPlayerHurt);
        UnloadSound(sfxDoor);
    }
    if (IsMusicValid(musicAmbient))
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
    loadCutsceneImages();
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
            updateMenu();
            break;

        case GameState::HOW_TO_PLAY:
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
                IsKeyPressed(KEY_SPACE)  || IsKeyPressed(KEY_BACKSPACE)) {
                state = GameState::TITLE;
                menuAnimTimer = 0.0f;
            }
            break;

        case GameState::CUTSCENE: {
            cutsceneTimer += GetFrameTime();
            const int TOTAL_SLIDES = 5;
            // Brief lock-out so the menu's ENTER press doesn't skip the first slide
            if (cutsceneTimer > 0.4f &&
                (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
                cutsceneSlide++;
                cutsceneTimer = 0.0f;
                if (cutsceneSlide >= TOTAL_SLIDES) {
                    currentLevel = 1;
                    init();
                    state = GameState::PLAYING;
                    DisableCursor();
                }
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                cutsceneSlide = 0;
                cutsceneTimer = 0.0f;
                currentLevel  = 1;
                init();
                state = GameState::PLAYING;
                DisableCursor();
            }
            break;
        }

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
                menuSelection = 0;
                menuAnimTimer = 0.0f;
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
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = GameState::TITLE;
                menuSelection = 0;
                menuAnimTimer = 0.0f;
                EnableCursor();
            }
            break;
    }
}

    
// ─── updatePlaying ────────────────────────────────────────────────────────────
void Game::updatePlaying() {

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
            drawMenu();
            break;

        case GameState::HOW_TO_PLAY:
            drawHowToPlay();
            break;

        case GameState::CUTSCENE:
            drawCutscene();
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

// ─── updateMenu ──────────────────────────────────────────────────────────────
void Game::updateMenu() {
    menuAnimTimer += GetFrameTime();

    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W))
        menuSelection = (menuSelection + 2) % 3;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        menuSelection = (menuSelection + 1) % 3;

    // Mouse hover
    const int itemY[3] = { SH/2 + 20, SH/2 + 80, SH/2 + 140 };
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; i++)
        if (mouse.y >= itemY[i] - 4 && mouse.y < itemY[i] + 46)
            menuSelection = i;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        switch (menuSelection) {
            case 0:
                cutsceneSlide = 0;
                cutsceneTimer = 0.0f;
                state = GameState::CUTSCENE;
                break;
            case 1:
                state = GameState::HOW_TO_PLAY;
                break;
            case 2:
                CloseWindow();
                break;
        }
    }
}

// ─── drawMenu ────────────────────────────────────────────────────────────────
void Game::drawMenu() {
    ClearBackground({ 4, 3, 3, 255 });

    // Atmospheric grid
    for (int x = 0; x < SW; x += 40)
        DrawLine(x, 0, x, SH, { 14, 10, 10, 255 });
    for (int y = 0; y < SH; y += 40)
        DrawLine(0, y, SW, y, { 14, 10, 10, 255 });

    // Vignette
    for (int i = 0; i < 120; i++) {
        unsigned char a = (unsigned char)(((120 - i) / 120.0f) * 160);
        DrawRectangle(0,      i,      SW, 1, { 0, 0, 0, a });
        DrawRectangle(0, SH-1-i,      SW, 1, { 0, 0, 0, a });
        DrawRectangle(     i, 0, 1, SH, { 0, 0, 0, a });
        DrawRectangle(SW-1-i, 0, 1, SH, { 0, 0, 0, a });
    }

    // Title
    float titlePulse = sinf(GetTime() * 1.2f) * 12 + 200;
    const char* title = "BLOOD SECTOR";
    int titleSize = SW > 1200 ? 88 : 72;
    int tw = MeasureText(title, titleSize);
    DrawText(title, SW/2 - tw/2 + 4, SH/2 - 190 + 4, titleSize, { 80, 0, 0, 120 });
    DrawText(title, SW/2 - tw/2,     SH/2 - 190,     titleSize,
             { (unsigned char)titlePulse, 18, 18, 255 });

    const char* sub = "A Game by CODE BROKERS";
    int sw2 = MeasureText(sub, 28);
    DrawText(sub, SW/2 - sw2/2, SH/2 - 90, 30, { 55, 40, 40, 255 });

    DrawRectangle(SW/2 - 140, SH/2 - 60, 280, 1, { 60, 20, 20, 200 });

    // Menu items
    const char* labels[3] = { "START GAME", "HOW TO PLAY", "EXIT GAME" };
    const int   itemY[3]  = { SH/2 + 20,   SH/2 + 80,    SH/2 + 140 };
    const int   itemSize  = 32;

    for (int i = 0; i < 3; i++) {
        bool selected = (menuSelection == i);
        int  lw       = MeasureText(labels[i], itemSize);

        if (selected) {
            float barPulse = sinf(GetTime() * 5.0f) * 0.1f + 0.9f;
            unsigned char ba = (unsigned char)(barPulse * 55);
            DrawRectangle(SW/2 - 160, itemY[i] - 6, 320, itemSize + 10,
                          { 120, 20, 20, ba });
            DrawRectangleLines(SW/2 - 160, itemY[i] - 6, 320, itemSize + 10,
                               { 185, 30, 30, 180 });

            DrawText(">", SW/2 - 150, itemY[i], itemSize, { 200, 40, 40, 255 });
            DrawText("<", SW/2 + 150 - MeasureText("<", itemSize),
                     itemY[i], itemSize, { 200, 40, 40, 255 });

            float lp = sinf(GetTime() * 6.0f) * 20 + 220;
            DrawText(labels[i], SW/2 - lw/2, itemY[i], itemSize,
                     { (unsigned char)lp, 30, 30, 255 });
        } else {
            DrawText(labels[i], SW/2 - lw/2, itemY[i], itemSize,
                     { 70, 50, 50, 200 });
        }
    }

    const char* hint = "W/S or UP/DOWN to navigate    ENTER to select";
    int hw = MeasureText(hint, 13);
    DrawText(hint, SW/2 - hw/2, SH - 30, 13, { 35, 28, 28, 200 });

    DrawText("v0.1  ALPHA", SW - 90, SH - 20, 11, { 30, 22, 22, 180 });
}

// ─── drawHowToPlay ───────────────────────────────────────────────────────────
void Game::drawHowToPlay() {
    ClearBackground({ 4, 3, 3, 255 });

    for (int x = 0; x < SW; x += 40)
        DrawLine(x, 0, x, SH, { 14, 10, 10, 255 });
    for (int y = 0; y < SH; y += 40)
        DrawLine(0, y, SW, y, { 14, 10, 10, 255 });

    int panW = 620, panH = 460;
    int panX = SW/2 - panW/2, panY = SH/2 - panH/2 - 10;
    DrawRectangle(panX, panY, panW, panH, { 8, 5, 5, 230 });
    DrawRectangleLines(panX,   panY,   panW,   panH,   { 80, 20, 20, 200 });
    DrawRectangleLines(panX+2, panY+2, panW-4, panH-4, { 40, 12, 12, 120 });

    const char* hdr = "HOW TO PLAY";
    int hdrW = MeasureText(hdr, 36);
    DrawText(hdr, SW/2 - hdrW/2, panY + 22, 36, { 185, 30, 30, 255 });
    DrawRectangle(panX + 30, panY + 70, panW - 60, 1, { 60, 20, 20, 180 });

    struct Row { const char* key; const char* action; };
    Row rows[] = {
        { "W / A / S / D",      "Move forward, strafe left, back, right" },
        { "ARROW KEYS",         "Move (alternate)"                        },
        { "MOUSE",              "Look left / right"                       },
        { "LEFT CLICK / SPACE", "Shoot"                                   },
        { "TAB",                "Toggle minimap"                          },
        { "F11",                "Toggle fullscreen"                       },
        { "ESC",                "Return to menu (game over screen)"       },
        { "R",                  "Restart from level 1"                    },
    };
    int rowCount = sizeof(rows) / sizeof(rows[0]);
    int startY   = panY + 90;
    int rowH     = 36;

    for (int i = 0; i < rowCount; i++) {
        int ry = startY + i * rowH;
        if (i % 2 == 0)
            DrawRectangle(panX + 12, ry, panW - 24, rowH - 4, { 18, 10, 10, 120 });
        DrawText(rows[i].key,    panX + 24,  ry + 8, 16, { 200, 140, 60, 255 });
        DrawText(rows[i].action, panX + 240, ry + 8, 16, { 130, 100, 100, 255 });
    }

    DrawRectangle(panX + 30, panY + panH - 80, panW - 60, 1, { 60, 20, 20, 160 });

    const char* obj = "OBJECTIVE:  Kill all enemies in each sector to advance.";
    int ow = MeasureText(obj, 15);
    DrawText(obj, SW/2 - ow/2, panY + panH - 65, 15, { 90, 130, 80, 255 });

    const char* warn = "Watch your ammo. The deeper you go, the worse it gets.";
    int ww = MeasureText(warn, 14);
    DrawText(warn, SW/2 - ww/2, panY + panH - 44, 14, { 70, 55, 55, 230 });

    float pulse = sinf(GetTime() * 3.0f) * 30 + 160;
    const char* back = "[ PRESS ENTER or ESC TO RETURN ]";
    int bw = MeasureText(back, 16);
    DrawText(back, SW/2 - bw/2, panY + panH + 16, 16,
             { (unsigned char)pulse, (unsigned char)(pulse/8),
               (unsigned char)(pulse/8), 255 });
}

// ─── drawCutscene ────────────────────────────────────────────────────────────
void Game::drawCutscene() {
    struct Slide { const char* heading; const char* body[4]; int lines; };

    static const Slide slides[] = {
        {
            "YEAR  2026",
            {
              "The OB1 Corporation built their research facility",
              "underneath the Rawalpindi city.",
              "Officially, it was a biotech lab working on medical advancements.",
              "Unofficially, it was a front for experiments to find immortality.",
            }, 4
        },
        {
            "PROJECT  ITTEFAQ",
            {
                "In late 2026, something went very wrong in Facility 9.",
                "A containment breach led to a rapid outbreak of a bioengineered pathogen.",
                "The infected rapidly lost their humanity, becoming aggressive and violent.",
                "The outbreak forced an immediate lockdown, trapping everyone inside.",
            }, 4
        },
        {
            "THREE  MONTHS  LATER",
            {
                "All communication with Facility 9 has been lost.",
                "The government has no choice but to send in a covert operative.",
                nullptr, nullptr
            }, 2
        },
        {
            "YOUR  ORDERS",
            {
              "You are Agent Ahmed Bilal.",
              "Infiltrate Facility 9. Locate any surviving personnel.",
              "Neutralize the biological outbreak at all costs.",
              "Command will deny this mission ever existed."
            }, 4
        },
        {
            "ENTER  THE  BREACH",
            {
              "Trust your instincts. Conserve your ammunition.",
              "Do not let them surround you.",
              "Do not stop moving.",
              nullptr
            }, 3
        },
    };

    const int TOTAL = 5;
    int idx = (cutsceneSlide < TOTAL) ? cutsceneSlide : TOTAL - 1;

    ClearBackground(BLACK);

    // ── Draw image fullscreen ─────────────────────────────────────────────────
    if (idx < 5 && cutsceneImages[idx].id > 0) {
        // Scale image to fill screen maintaining aspect ratio
        float imgW = (float)cutsceneImages[idx].width;
        float imgH = (float)cutsceneImages[idx].height;
        float scale = fmaxf((float)SW / imgW, (float)SH / imgH);
        int drawW = (int)(imgW * scale);
        int drawH = (int)(imgH * scale);
        int drawX = (SW - drawW) / 2;
        int drawY = (SH - drawH) / 2;
        DrawTextureEx(cutsceneImages[idx],
                      { (float)drawX, (float)drawY },
                      0.0f, scale, WHITE);
    } else {
        // Fallback — dark background with scanlines
        for (int y = 0; y < SH; y += 4)
            DrawRectangle(0, y, SW, 1, { 6, 4, 4, 255 });
    }

    // ── Dark gradient at bottom for text readability ───────────────────────────
    int gradH = SH / 3;
    for (int i = 0; i < gradH; i++) {
        unsigned char a = (unsigned char)(((float)(gradH - i) / gradH) * 220);
        DrawRectangle(0, SH - gradH + i, SW, 1, { 0, 0, 0, 220 });
    }

    // ── Dark gradient at top for counter readability ──────────────────────────
    for (int i = 0; i < 60; i++) {
        unsigned char a = (unsigned char)(((float)(60 - i) / 60.0f) * 180);
        DrawRectangle(0, i, SW, 1, { 0, 0, 0, a });
    }

    // ── Top UI ────────────────────────────────────────────────────────────────
    const char* counter = TextFormat("%d / %d", idx + 1, TOTAL);
    DrawText(counter, SW - MeasureText(counter, 20) - 24, 16, 20, { 50, 30, 30, 200 });
    DrawText("ESC - SKIP ALL", 22, 16, 45, { 40, 28, 28, 180 });

    // ── Heading (bottom area, above body text) ────────────────────────────────
    int textAreaTop = SH - 220;

    float hp = sinf(GetTime() * 1.5f) * 10 + 210;
    int   hSize = 36;
    const char* heading = slides[idx].heading;
    int   hw2 = MeasureText(heading, hSize);
    DrawText(heading, SW/2 - hw2/2, textAreaTop, hSize,
             { (unsigned char)hp, 22, 22, 255 });

    // Divider line under heading
    DrawRectangle(SW/2 - 300, textAreaTop + hSize + 6, 600, 1, { 80, 20, 20, 180 });

    // ── Body text lines with staggered fade-in ────────────────────────────────
    int lineSize   = 25;
    int lineGap    = 28;
    int bodyStartY = textAreaTop + hSize + 18;

    for (int l = 0; l < slides[idx].lines; l++) {
        if (!slides[idx].body[l]) break;
        float lineDelay = l * 0.22f;
        float alpha01   = (cutsceneTimer - lineDelay) / 0.35f;
        if (alpha01 < 0.0f) alpha01 = 0.0f;
        if (alpha01 > 1.0f) alpha01 = 1.0f;
        unsigned char a = (unsigned char)(alpha01 * 210);
        int lw2 = MeasureText(slides[idx].body[l], lineSize);
        DrawText(slides[idx].body[l], SW/2 - lw2/2,
                 bodyStartY + l * lineGap, lineSize,
                 { a, (unsigned char)(a * 0.78f), (unsigned char)(a * 0.78f), 255 });
    }

    // ── Advance prompt ────────────────────────────────────────────────────────
    if (cutsceneTimer > 1.0f) {
        float pulse = sinf(GetTime() * 3.5f) * 35 + 150;
        const char* prompt = (idx < TOTAL - 1)
                             ? "[ PRESS ENTER TO CONTINUE ]"
                             : "[ PRESS ENTER TO BEGIN ]";
        int pw = MeasureText(prompt, 15);
        DrawText(prompt, SW/2 - pw/2, SH - 28, 15,
                 { (unsigned char)pulse, (unsigned char)(pulse/8),
                   (unsigned char)(pulse/8), 255 });
    }
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
    for (int i = 0; i < 5; i++)
    if (cutsceneImages[i].id > 0) UnloadTexture(cutsceneImages[i]);
    delete player;   player   = nullptr;
    delete renderer; renderer = nullptr;
    for (Enemy* e : enemies) delete e;
    enemies.clear();
}

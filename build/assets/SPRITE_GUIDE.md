SHADOW BREACH — Sprite Guide
============================
Tool: Piskel (piskel.com) — free, browser-based pixel art editor
Format: PNG, transparent background

All sprites are drawn with pixel art style. Keep colours dark/desaturated —
the renderer applies its own fog and depth tinting over them.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
WALKER  →  walker.png   (32 × 48 px)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Classic shambling humanoid silhouette.

  Head (top 10px):  rough round shape, dark skin (#6E3A2A)
  Torso (next 20px): ragged dark jacket (#2A1A1A), slightly wider than head
  Arms: hang slightly away from torso, same colour
  Legs (bottom 18px): dark grey trousers (#1A1A1A), slightly apart

  Eyes: 2 white pixels on head
  Mouth: 2 dark red pixels (#6E0000)
  Outline: 1px black border around entire sprite

  Idle frame = standing upright
  If you make 2 frames (walk_1 + walk_2), name them walker_1.png / walker_2.png
  Frame 2: shift left foot forward slightly, right foot back

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
STALKER  →  stalker.png   (28 × 52 px)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Taller and thinner than Walker — unsettling proportions.

  Head (10px): elongated oval, pale (#C8A882), no neck
  Torso (22px): very narrow, dark purple-black (#1A0028)
  Arms: unnaturally long, reach past hip level, thin
  Legs (20px): long, narrow, no feet (fades into nothing)

  Eyes: 2 bright purple pixels (#B400FF) that glow
  No visible mouth
  Frozen state (player looking at it): add a white outline 1px around whole sprite

  If you make 2 frames:
  stalker_1.png: arms at sides
  stalker_2.png: arms raised slightly outward

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
BOSS   →  boss.png   (48 × 64 px)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Large, hunched, heavily built. Clearly "final enemy" scale.

  Head (12px): bulky, dark red (#4A0000), no clear face features
  Torso (26px): wide and thick (#2A0000), roughly square shape
  Arms: massive, low, almost dragging
  Legs (26px): wide apart, dark, no detail needed

  Eyes: 2 bright red pixels (#FF2200), larger than Walker
  Charging state: add yellow outline (#FFCC00) 1px border
  Lunging state: add orange outline (#FF6600) 1px border

  If you make 2 frames:
  boss_idle.png: arms hanging low
  boss_charge.png: arms raised, body leaning forward

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GUN OVERLAY  →  gun_idle.png + gun_fire.png   (128 × 64 px each)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Drawn in BOTTOM-CENTRE of screen. Only visible in FPP.
  Most of sprite should be transparent — only the gun is drawn.

  gun_idle.png:
    Simple top-down pistol / handgun, dark metal (#2A2A2A to #4A4A4A)
    Barrel pointing UP (toward screen top = away from player)
    Positioned in bottom-right area of the 128×64 canvas
    Grip at bottom-right, barrel at top-centre-right
    ~40px wide, ~50px tall, rest transparent

  gun_fire.png:
    Same as idle but:
    Add a 6×6 px yellow/white flash (#FFFF88 to white) at barrel tip
    Barrel can be slightly offset (recoil: shift 2px down, 1px right)

NOTE: Sprite loading is NOT yet wired into the code by default — the renderer
currently draws procedural shapes. To enable sprites you would add:
  Texture2D texWalker = LoadTexture("assets/sprites/walker.png");
...and draw with DrawBillboard() in Renderer.cpp drawSprites().
This is a clean extension task if you have time.

# MECH472 — Development Summary

Summary of what is implemented so far and conventions to keep in mind. See [PRD and TDD.md](PRD%20and%20TDD.md) for requirements and [Phased Dev Plan.md](Phased%20Dev%20Plan.md) for the full roadmap.

---

## 1. What we’re building

A **2D simulation** (not a game) that models the autonomous logic for a differential-drive robot with an independent laser turret. It will validate:

- Navigation and obstacle avoidance (APF)
- Line-of-sight (LoS) targeting
- Attacker (hunt) vs Defender (evade) behavior

**In scope:** 2D sim, state machines, LoS, APF, coordinate transforms, visual debugging.  
**Out of scope:** Motor control (PID, wheel velocities), camera hardware, computer vision.

---

## 2. What’s done so far

| Phase | Status | What was added |
|-------|--------|----------------|
| **1** | Done | Minimal runnable shell: `sim_main.cpp`, `world` class, `global_data.h`, main loop `clear()` → `Update()` → `Draw()` → exit on Escape. Project uses 2D graphics lib from `LIbraries/`, Win32 only, static CRT. |
| **2** | Done | 6 ft × 6 ft battlefield (72 × 72 inches). Camera→screen transform (`CameraToScreen`). Field border and 1 ft grid. Window 720×720 px (10 px/inch). See [DirectX_Window_Size.md](DirectX_Window_Size.md). |
| **3** | Done | Obstacles: `struct obstacle` (X, Y, R in inches, `isActive`). Fixed array `Obstacles[N_OBSTACLES_MAX]`. Obstacles drawn as circles via `line()` with camera→screen transform. |
| **4** | Done | Robots in `robot.h` / `robot.cpp`: `struct robot` (X, Y, theta_chassis, theta_laser, **laserOn**, target_x, target_y). Body = 12"×7" filled rectangle (two `triangle()` calls). Turret circle ≤ 1/3 body width; laser line (amber/yellow) only when `laserOn`. Defender has laser off. `Draw(robot, world, isAttacker)`; world exposes `CameraToScreen`. |
| **5** | Done | Q key shuffle: `world::Shuffle()` repositions obstacles and robots. One-shot key handling (Q edge). **Shuffle for visualization:** robots spawn at **opposite corners** (Attacker near (6,6), Defender near (66,66)); **one obstacle** (Obstacles[0]) placed at midpoint between them so LoS is blocked and Defender has cover. Remaining obstacles randomized without overlapping obstacle 0 or robots. |
| **6** | Done | Line of Sight: ray–circle LoS from Attacker to Defender. `los_clear`, `blocking_obstacle_index`. Draw segment Attacker→Defender: **green** if clear, **red** if blocked. Attacker turret aims at Defender. |
| **6+** | Done | Turret limit: 0°–180° relative to chassis, 90° = straight forward; ±90° from chassis (`TurretHalfRangeDeg`). |
| **7** | Done | Attacker logic: when **LoS clear**, target = Defender. When **LoS blocked**, target = **waypoint** to the side of the blocking obstacle (perpendicular to Attacker–obstacle, chosen closer to Defender) so Attacker navigates around. Turret aims at Defender. Chassis heading comes from APF (see Phase 9). |
| **8** | Done | Defender target = **shadow** behind an obstacle (vector Attacker→obstacle, extend by R + robot R + buffer). Prefer blocking obstacle’s shadow if valid. **Validation:** shadow rejected if inside another obstacle or too close to walls (`RobotRadiusInches` margin). **Hold when safe:** if LoS blocked and Defender within `ShadowArrivalInches` of shadow target, target = current position. Cyan debug marker at Defender target. |
| **9** | Done | **Non-holonomic APF:** desired direction from total force; chassis turns toward `desired_theta`; robot moves **forward along chassis only** (no sideways). Attractive + obstacle repulsion (capped) + **tangential** component (orbit around obstacles, break deadlock) + **wall repulsion** + **robot–robot repulsion**. **Two-radius model:** `APFRadiusInches` (4") for repulsion planning (tighter gaps); `RobotRadiusInches` (6") for post-move collision check and spawn. Post-move reject if new position would penetrate an obstacle. |

**Next:** Phase 10 (Polish: HUD, tune constants).

---

## 3. Things to keep in mind

### 3.1 Units and coordinate system

- **All simulation units are in inches.** Positions (X, Y), radii (R), and distances use inches. Only at draw time do we convert to pixels.
- **Battlefield:** 6 ft × 6 ft = 72 × 72 inches. Bounds: `[0, 72]` in X and Y (camera space).
- **Camera space (PRD):** Origin (0, 0) is **top-right**. X increases **left**, Y increases **down**.
- **Screen (pixels):** Origin top-left. Transform:
  - `X_screen = WindowWidth - (x_inches * PixelsPerInch)`
  - `Y_screen = y_inches * PixelsPerInch`
- **Window:** 720 × 720 pixels ⇒ `PixelsPerInch = 10`. DirectX window must be set to 720×720 (see [DirectX_Window_Size.md](DirectX_Window_Size.md)).

### 3.2 Build and project

- **Platform:** **Win32 only** (x86). The provided `2D_graphics_ver1.lib` is 32-bit; do not build for x64.
- **Runtime library:** **Static CRT** (`/MTd` Debug, `/MT` Release). The prebuilt lib uses static CRT; the vcxproj is set to match. Do not switch to `/MDd`/`/MD` or you get link errors.
- **Libraries:** Only `2D_graphics.h` and `2D_graphics_ver1.lib` from the `LIbraries/` folder. No STL in simulation logic; no timer/ran unless you add them later (shuffle will use `rand()`/`srand()`).
- **Run order:** Start **DirectX_window.exe** first, then run the simulation .exe.

### 3.3 Code and architecture

- **Pattern:** Single `world` class; flat source layout; frame order `clear()` → `World.Update(dt)` → `World.Draw()` → input/exit → `update()`.
- **Naming (AI_REFERENCE):** Class names `snake_case` (e.g. `world`), methods `PascalCase` (e.g. `Update`, `Draw`). Constants in `global_data.h`.
- **Memory:** Fixed-size arrays; `isActive` to enable/disable entities. No dynamic allocation of entities in the loop; world owns all and deletes only in destructor.
- **Graphics API:** Use only what’s in `2D_graphics.h` (`clear`, `update`, `line`, `triangle`, `text`, `create_sprite`, `draw_sprite`, `KEY`). Do not modify `2D_graphics.h`.
- **Robot mode:** `robot.laserOn` true = attack mode (draw laser, aim at target); false = defense mode (no laser drawn). `robot.target_x`, `robot.target_y` = target for APF (Attacker → Defender or waypoint when LoS blocked; Defender → shadow).
- **Motion model:** Differential drive (non-holonomic). Robots turn toward APF desired direction, then move forward along `theta_chassis` only; speed scaled by alignment (`cos(angle_diff)`). No sideways motion.
- **Collision model:** Planning uses `APFRadiusInches` (4") for repulsion so the 12"×7" body can attempt tighter gaps; post-move collision and spawn use `RobotRadiusInches` (6") bounding circle.
- **Turret limit:** Laser turret 0°–180° relative to chassis (90° = forward); clamp via `TurretHalfRangeDeg`.

### 3.4 Input

- **Q:** Reserved for **shuffle** (Phase 5). Do not use Q to exit.
- **Escape:** Exit simulation.

---

## 4. File layout (current)

```
src/MECH 472 Project/
  sim_main.cpp       — entry point; Q = shuffle (one-shot), Escape = exit
  world.cpp / world.h — world class, CameraToScreen, obstacles, Attacker/Defender, los_clear, blocking_obstacle_index; Shuffle() = opposite corners + one obstacle between; Update() = LoS, Attacker waypoint when blocked, Defender shadow (validated), non-holonomic APF for both; Draw() = field, obstacles, robots, LoS line, Defender target marker
  robot.cpp / robot.h — robot struct, Draw(robot, world, isAttacker); 12"×7" body, turret circle, amber laser if laserOn
  global_data.h      — Window/World constants, RobotRadiusInches, APFRadiusInches, WaypointClearanceInches, ShadowBufferInches, ShadowArrivalInches, APF/tangential/wall/repulsion constants, ChassisTurnRateRadPerFrame, RobotSpeedInchesPerFrame
  LIbraries/
    2D_graphics.h
    2D_graphics_ver1.lib
```

Build output: `Debug/MECH 472 Project.exe` (or `Release/`). Compiler artifacts in `.gitignore`.

---

## 5. Quick run checklist

1. Set DirectX window to **720 × 720** (see [DirectX_Window_Size.md](DirectX_Window_Size.md)).
2. Start **DirectX_window.exe**.
3. Build **MECH 472 Project** (Debug or Release, **Win32**).
4. Run **MECH 472 Project.exe**.
5. You should see: 6 ft×6 ft field, grid, orange obstacles, Attacker (blue) and Defender (green) at **opposite corners** with **one obstacle between them** (LoS blocked). **Cyan circle** = Defender’s shadow target. **Green/red** line = LoS (clear/blocked). Robots **turn then drive forward** (no sideways drift). Press **Q** to reshuffle (same corner layout + obstacle between); **Escape** to exit.

---

## 6. Reference docs

- [PRD and TDD.md](PRD%20and%20TDD.md) — product and technical requirements
- [Phased Dev Plan.md](Phased%20Dev%20Plan.md) — phased plan with tests
- [DirectX_Window_Size.md](DirectX_Window_Size.md) — window size for 6 ft×6 ft
- [../agent/AI_REFERENCE.md](../agent/AI_REFERENCE.md) — tech stack, style, patterns for AI/developers

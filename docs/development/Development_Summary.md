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
| **3** | Done | Obstacles: `struct obstacle` (X, Y, R in inches, `isActive`). Fixed array `Obstacles[N_OBSTACLES_MAX]`. Five obstacles drawn as circles via `line()` with camera→screen transform. |
| **4** | Done | Robots in `robot.h` / `robot.cpp`: `struct robot` (X, Y, theta_chassis, theta_laser, **laserOn**). Body = 12"×7" filled rectangle (two `triangle()` calls). Turret circle ≤ 1/3 body width; laser line only when `laserOn` (attack mode), extended way beyond body. Defender has laser off. `Draw(robot, world, isAttacker)`; world exposes `CameraToScreen`. |
| **5** | Done | Q key shuffle: `world::Shuffle()` repositions all active obstacles and both robots. `srand()` in world ctor; `rand()` in Shuffle. Obstacles avoid overlap; robots avoid obstacles and each other (`RobotRadiusInches`). One-shot key handling in `sim_main.cpp` (Q edge triggers one shuffle). |
| **6** | Done | Line of Sight: ray–circle LoS from Attacker to Defender. In `Update()` compute `los_clear` (closest point on segment to each obstacle center; if distance < R, blocked). In `Draw()` draw segment Attacker→Defender: green if clear, red if blocked. |

**Next:** Phase 7 (Attacker logic).

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
- **Robot mode:** `robot.laserOn` true = attack mode (draw laser, later triggers aim/target logic); false = defense mode (no laser drawn, different logic).

### 3.4 Input

- **Q:** Reserved for **shuffle** (Phase 5). Do not use Q to exit.
- **Escape:** Exit simulation.

---

## 4. File layout (current)

```
src/MECH 472 Project/
  sim_main.cpp       — entry point, main loop
  world.cpp / world.h — world class, CameraToScreen (public), obstacles, Attacker/Defender, draw
  robot.cpp / robot.h — robot struct, Draw(robot, world, isAttacker); body, turret circle, laser line if laserOn
  global_data.h      — WindowWidth/Height, World*Inches, PixelsPerInch, N_OBSTACLES_MAX, robot/turret/laser constants
  LIbraries/
    2D_graphics.h
    2D_graphics_ver1.lib
```

Build output: `Debug/MECH 472 Project.exe` (or `Release/`). Compiler artifacts (e.g. `x64/`, `.vs/`, `MECH 472 Project/`, `Debug/`, `Release/`) are in `.gitignore`.

---

## 5. Quick run checklist

1. Set DirectX window to **720 × 720** (see [DirectX_Window_Size.md](DirectX_Window_Size.md)).
2. Start **DirectX_window.exe**.
3. Build **MECH 472 Project** (Debug or Release, **Win32**).
4. Run **MECH 472 Project.exe**.
5. You should see: 6 ft×6 ft field, grid every 1 ft, 5 orange obstacle circles, two robots (Attacker with extended red laser line, Defender without laser). Press **Escape** to exit.

---

## 6. Reference docs

- [PRD and TDD.md](PRD%20and%20TDD.md) — product and technical requirements
- [Phased Dev Plan.md](Phased%20Dev%20Plan.md) — phased plan with tests
- [DirectX_Window_Size.md](DirectX_Window_Size.md) — window size for 6 ft×6 ft
- [../agent/AI_REFERENCE.md](../agent/AI_REFERENCE.md) — tech stack, style, patterns for AI/developers

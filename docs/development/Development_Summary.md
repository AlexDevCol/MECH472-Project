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
| **3** | Done | Obstacles: `struct obstacle` (X, Y, R in inches, `isActive`). Fixed array `Obstacles[N_OBSTACLES_MAX]`. Eight obstacles drawn as circles via `line()` with camera→screen transform. |

**Next:** Phase 4 (robots draw-only), then Phase 5 (Q = shuffle).

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
- **Graphics API:** Use only what’s in `2D_graphics.h` (`clear`, `update`, `line`, `text`, `create_sprite`, `draw_sprite`, `KEY`). Do not modify `2D_graphics.h`.

### 3.4 Input

- **Q:** Reserved for **shuffle** (Phase 5). Do not use Q to exit.
- **Escape:** Exit simulation.

---

## 4. File layout (current)

```
src/MECH 472 Project/
  sim_main.cpp       — entry point, main loop
  world.cpp / world.h — world class, CameraToScreen, obstacles, draw
  global_data.h      — WindowWidth/Height, World*Inches, PixelsPerInch, N_OBSTACLES_MAX
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
5. You should see: 6 ft×6 ft field, grid every 1 ft, 8 orange obstacle circles. Press **Escape** to exit.

---

## 6. Reference docs

- [PRD and TDD.md](PRD%20and%20TDD.md) — product and technical requirements
- [Phased Dev Plan.md](Phased%20Dev%20Plan.md) — phased plan with tests
- [DirectX_Window_Size.md](DirectX_Window_Size.md) — window size for 6 ft×6 ft
- [../agent/AI_REFERENCE.md](../agent/AI_REFERENCE.md) — tech stack, style, patterns for AI/developers

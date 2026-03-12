# PRD & TDD: Autonomous Combat Robot Simulation

## Part 1: Product Requirements Document (PRD)

### 1. Objective

To develop a 2D simulation in C++ that models the autonomous logic ("brain") for a differential drive robot equipped with an independent laser turret. The simulation will validate navigation, obstacle avoidance, Line of Sight (LoS) targeting, and evasion tactics before deploying the code to physical hardware.

### 2. Scope & Roles

* **In Scope:** 2D physics simulation, Attacker/Defender state machines, LoS calculations, Artificial Potential Field (APF) pathfinding, coordinate transformations, and visual debugging (drawing paths/lasers).
* **Out of Scope (Handled by Teammates):** Low-level motor control (PID, wheel velocities $v_L$ and $v_R$), physical camera hardware interfacing, and real-time computer vision processing.

### 3. Inputs & Outputs (The Handoff)

* **Simulated Input (Mocking the Camera):** * The environment uses a **Top-Right Origin** coordinate system.
* Data received: `Robot Pose (X, Y, Theta)`, `Enemy Pose (X, Y, Theta)`, and `Obstacles Array [X, Y, Radius]`.


* **Logic Output (To the Motor Controller):**
* `Target X`, `Target Y` (Where the chassis needs to go).
* `Target Chassis Theta` (Required heading to reach the target).
* `Target Laser Theta` (Independent angle to aim the laser).



### 4. Core Behaviors

* **Attacker Mode:** Must actively hunt the Defender. If LoS is clear, aim the laser and fire. If LoS is blocked by an obstacle, navigate around the obstacle to re-establish LoS.
* **Defender Mode:** Must actively evade the Attacker. It calculates "shadows" behind obstacles relative to the Attacker's position and navigates to those safe zones to break LoS.

---

## Part 2: Technical Design Document (TDD)

### 1. Tech Stack & Architecture

* **Language:** C++ (No STL containers for game logic; raw arrays and pointers).
* **Graphics:** Custom `2D_graphics.h` library provided by the professor.
* **Design Pattern:** Single `world` class pattern. Flat source layout. Frame order: `clear()` $\rightarrow$ `World.Update(dt)` $\rightarrow$ `World.Draw()` $\rightarrow$ input (Q shuffle, exit) $\rightarrow$ `update()`.
* **Memory Management:** Fixed-size arrays initialized in the `world` constructor. Objects use an `isActive` boolean flag to toggle state rather than runtime dynamic allocation/deletion.

### 2. Coordinate System Transformation

Because the physical camera data places $(0,0)$ at the top-right, and the graphics library expects standard top-left projection, all internal logic will run in the camera's space. We will only transform coordinates at the moment of rendering:


$$X_{screen} = \text{WindowWidth} - X_{camera}$$

$$Y_{screen} = Y_{camera}$$

### 3. Core Algorithms

#### A. Line of Sight (Ray-Circle Intersection)

To determine if a laser shot is clear or blocked by an obstacle, we will use point-to-line distance math. Let $\vec{d}$ be the ray from the Attacker to the Defender. For each active obstacle at center $C$ with radius $R$:

1. Find the closest point $Q$ on the segment $\vec{d}$ to $C$.
2. If $\|Q - C\| < R$, the LoS is blocked (laser hits the obstacle).

#### B. Defender Evasion (Obstacle Shadows)

To hide the Defender, we calculate a target coordinate strictly behind an obstacle relative to the Attacker:

1. Create a vector $\vec{V}$ from the Attacker's $(X,Y)$ to the Obstacle's $(X,Y)$.
2. Normalize $\vec{V}$ and multiply it by a scalar (Obstacle Radius + Robot Radius + Buffer).
3. Add this extended vector to the Obstacle's $(X,Y)$ to get the "Shadow Coordinate". This becomes the Defender's target destination.

#### C. Pathfinding & Obstacle Avoidance (APF)

We will use Artificial Potential Fields to navigate both robots smoothly without hitting obstacles.

* **Attractive Force:** A vector pulling the robot toward its target (Defender for the Attacker; Shadow Coordinate for the Defender).
* **Repulsive Force:** Vectors pushing the robot away from any obstacle whose distance is less than a defined safety threshold. Repulsion magnitude is capped to avoid instability when very close. A *tangential* component is added to repulsion so the robot is nudged to orbit around obstacles (reduces deadlock when on opposite sides). *Wall repulsion* pushes robots away from field boundaries. *Robot–robot repulsion* keeps Attacker and Defender from overlapping.
* **Result:** The sum of these vectors yields a desired heading; the chassis turns toward it and the robot moves forward along chassis only (non-holonomic).

**Implementation decisions (current):**

* **Non-holonomic motion:** Robots are differential-drive; they cannot move sideways. The APF total force gives a desired heading; the chassis turns toward that heading at a limited rate, and the robot moves *forward along the chassis heading only*, with speed scaled by how well the chassis aligns with the desired direction. This avoids unrealistic sideways drift.
* **Attacker when LoS blocked:** Instead of targeting the Defender directly (which points into the blocking obstacle), the Attacker’s target is set to a *waypoint* to the side of the blocking obstacle (perpendicular to the Attacker–obstacle line, on the side closer to the Defender). The Attacker then orbits toward the waypoint; when LoS clears, the target switches back to the Defender.
* **Defender shadow validation:** A shadow candidate is accepted only if it is in bounds, not too close to walls (to avoid cornering), and *not inside another obstacle*. If the Defender is already close to its shadow and LoS is blocked, the target is set to the Defender’s current position (hold) to avoid unnecessary motion.
* **Collision and planning radii:** The physical robot body is a 12"×7" rectangle. For *collision* (spawn overlap, post-move penetration check) we use a bounding circle radius `RobotRadiusInches` (6"). For *APF repulsion* we use a smaller `APFRadiusInches` (4") so the robot attempts tighter gaps that the box can fit through; the hard collision check still prevents penetration.
* **Shuffle for visualization:** On shuffle (Q key), robots are placed at opposite corners of the field and one obstacle is placed on the segment between them so the Defender starts with LoS blocked and the scenario is easy to reproduce.

### 4. Game Loop Structure (`world.cpp`)

* **`world::update(dt)`:**
* Parse mock camera data (or keyboard inputs for manual override).
* Run LoS checks (Ray-Circle intersection).
* Execute State Machine (Attacker logic vs. Defender logic).
* Calculate APF vectors to update the Target Coordinates and Thetas.


* **`world::draw()`:**
* Draw the field/background.
* Draw obstacles using `line()` in a radial loop to approximate circles.
* Draw the robots (Attacker and Defender) with their respective chassis headings.
* Draw the independent laser turrets.
* Draw the laser beam (Green line if LoS clear, Red line if blocked).

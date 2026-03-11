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
* **Design Pattern:** Single `world` class pattern. Flat source layout. `clear()` $\rightarrow$ `update()` $\rightarrow$ `draw()` frame sequence.
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
* **Repulsive Force:** Vectors pushing the robot away from any obstacle whose distance is less than a defined safety threshold.
* **Result:** The sum of these vectors yields the final `Target X` and `Target Y`.

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

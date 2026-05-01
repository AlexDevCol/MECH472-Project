// Simulation constants — battlefield and window

// Battlefield: 6 ft x 6 ft in inches (all simulation units in inches)
#define InchesPerFoot       12
#define BattlefieldFeet    6
#define WorldWidthInches   (BattlefieldFeet * InchesPerFoot)   // 72
#define WorldHeightInches  (BattlefieldFeet * InchesPerFoot)   // 72

// DirectX window size in pixels — set to match window_size.txt in DirectX_window project.
// Use 720 x 720 for 10 pixels per inch (6 ft x 6 ft).
#define WindowWidth  720.0
#define WindowHeight 720.0

// Scale: pixels per inch (world is in inches)
#define PixelsPerInch  (WindowWidth / WorldWidthInches)

// Obstacles: fixed array size (no STL)
#define N_OBSTACLES_MAX  8

// Robot body and turret (inches)
#define RobotBodyLengthInches  12
#define RobotBodyWidthInches   7
// Turret circle at most 1/3 of body width (7/3 ~ 2.33)
#define TurretCircleRadiusInches  2.0
// Laser line length (extends beyond body to simulate laser)
#define LaserLineLengthInches  72
// Robot radius for spawn rejection (obstacle overlap check)
#define RobotRadiusInches  6
// Turret limit: 0..180 deg relative to chassis, 90 deg = straight forward; ±90 deg each side
#define TurretForwardDeg  90
#define TurretHalfRangeDeg  90
// Phase 7: max chassis turn per frame (rad) when stepping toward target
#define ChassisTurnRateRadPerFrame  0.06
// Phase 8: buffer (inches) for Defender shadow point behind obstacle (obstacle R + robot R + buffer)
#define ShadowBufferInches  3
// Phase 9: APF pathfinding and motion
#define APFRepulsiveGain       80.0   // repulsive force scale
#define APFSafetyMarginInches  6.0    // extra margin beyond obstacle R + robot R for repulsion influence
#define APFRadiusInches  4.0   // smaller radius for APF repulsion only (robot can fit tighter gaps); collision check still uses RobotRadiusInches
#define WaypointClearanceInches  4.0  // Attacker waypoint around blocking obstacle: clearance beyond obstacle R + robot R
#define RobotSpeedInchesPerFrame  0.18  // movement step per frame (dt not used); reduced to avoid vibration
#define MaxRepulsiveForce      15.0   // cap per-obstacle/robot repulsion magnitude to prevent vibration
#define RobotRobotRepelDistInches  14.0  // distance below which Attacker and Defender repel each other
// Non-holonomic APF: tangential force to orbit around obstacles (break deadlock)
#define APFTangentialFraction  0.6
// Wall repulsion so robots don't get pushed to edges
#define WallRepelDistInches  10.0
#define WallRepelGain  40.0
// Defender: hold when already at shadow and LoS blocked; reject shadows too close to walls
#define ShadowArrivalInches  4.0
// Phase 10: committed contour (avoid free-spinning around obstacles)
#define ContourStallThreshold     60    // frames before switching sides (~1-2 sec)
#define ContourArrivalRadius      3.0   // inches: "close enough" to waypoint
#define ContourProgressEpsilon    0.5   // min distance decrease per check window
// Phase 10: defender prefers shadows in attacker's blind spot
#define BlindSpotBonus  0.4   // multiplier on distance (lower = more attractive)
// Phase 10: defender stall detection when heading to shadow
#define DefenderStallThreshold    80    // frames without progress before trying next shadow
#define DefenderShadowProgressEpsilon  0.3
// Phase 10: gap feasibility when choosing contour side (min passage width for robot)
#define MinPassageWidthInches  (RobotBodyWidthInches + 2.0)

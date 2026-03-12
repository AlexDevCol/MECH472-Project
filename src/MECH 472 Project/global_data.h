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
#define N_OBSTACLES_MAX  20

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

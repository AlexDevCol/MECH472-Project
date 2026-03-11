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

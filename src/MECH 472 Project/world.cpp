// world — simulation container implementation

#include "world.h"
#include "global_data.h"
#include "2D_graphics.h"

world::world() {
	// Phase 1–2: no entities yet
}

world::~world() {
	// Phase 1–2: nothing to delete
}

void world::CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const {
	// PRD: top-right origin in camera space; screen top-left
	// X_screen = WindowWidth - X_camera (in pixels), Y_screen = Y_camera
	double px = x_inches * PixelsPerInch;
	double py = y_inches * PixelsPerInch;
	out_x = WindowWidth - px;
	out_y = py;
}

void world::Update(double dt) {
	// Phase 2: no simulation logic yet
	(void)dt;
}

void world::Draw() {
	// Field border: 6 ft x 6 ft in camera space (0,0) to (72, 72) inches
	double x[5], y[5];
	double sx, sy;
	CameraToScreen(0, 0, sx, sy);                    x[0] = sx; y[0] = sy;
	CameraToScreen(WorldWidthInches, 0, sx, sy);     x[1] = sx; y[1] = sy;
	CameraToScreen(WorldWidthInches, WorldHeightInches, sx, sy); x[2] = sx; y[2] = sy;
	CameraToScreen(0, WorldHeightInches, sx, sy);    x[3] = sx; y[3] = sy;
	x[4] = x[0]; y[4] = y[0];
	line(x, y, 5, 0.2, 0.2, 0.2);

	// Grid every 1 ft (12 inches)
	const int n = (int)(WorldWidthInches / InchesPerFoot) + 1;
	for (int i = 0; i < n; i++) {
		double in = (double)(i * InchesPerFoot);
		// Vertical line at x = in
		CameraToScreen(in, 0, x[0], y[0]);
		CameraToScreen(in, WorldHeightInches, x[1], y[1]);
		line(x, y, 2, 0.35, 0.35, 0.35);
		// Horizontal line at y = in
		CameraToScreen(0, in, x[0], y[0]);
		CameraToScreen(WorldWidthInches, in, x[1], y[1]);
		line(x, y, 2, 0.35, 0.35, 0.35);
	}
}

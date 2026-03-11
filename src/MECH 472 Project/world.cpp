// world — simulation container implementation

#include <cmath>
#include "world.h"
#include "global_data.h"
#include "2D_graphics.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

world::world() {
	N_obstacles = 8;
	// Fixed initial positions (inches) and radii — all active for Phase 3
	Obstacles[0].X = 18;  Obstacles[0].Y = 18;  Obstacles[0].R = 4;  Obstacles[0].isActive = true;
	Obstacles[1].X = 54;  Obstacles[1].Y = 24;  Obstacles[1].R = 5;  Obstacles[1].isActive = true;
	Obstacles[2].X = 36;  Obstacles[2].Y = 48;  Obstacles[2].R = 3;  Obstacles[2].isActive = true;
	Obstacles[3].X = 12;  Obstacles[3].Y = 54;  Obstacles[3].R = 4;  Obstacles[3].isActive = true;
	Obstacles[4].X = 60;  Obstacles[4].Y = 60;  Obstacles[4].R = 3;  Obstacles[4].isActive = true;
	Obstacles[5].X = 24;  Obstacles[5].Y = 36;  Obstacles[5].R = 5;  Obstacles[5].isActive = true;
	Obstacles[6].X = 48;  Obstacles[6].Y = 12;  Obstacles[6].R = 3;  Obstacles[6].isActive = true;
	Obstacles[7].X = 42;  Obstacles[7].Y = 42;  Obstacles[7].R = 4;  Obstacles[7].isActive = true;
	for (int i = N_obstacles; i < N_OBSTACLES_MAX; i++)
		Obstacles[i].isActive = false;
}

world::~world() {
	// Phase 3: fixed array, nothing to delete
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

	// Obstacles: draw as circles (radial line loop), camera→screen transform
	const int circle_segs = 32;
	double cx[circle_segs + 1], cy[circle_segs + 1];
	for (int i = 0; i < N_OBSTACLES_MAX; i++) {
		if (!Obstacles[i].isActive) continue;
		double ox = Obstacles[i].X, oy = Obstacles[i].Y, r = Obstacles[i].R;
		for (int s = 0; s <= circle_segs; s++) {
			double a = (double)s / (double)circle_segs * 2.0 * M_PI;
			double px = ox + r * cos(a);
			double py = oy + r * sin(a);
			CameraToScreen(px, py, cx[s], cy[s]);
		}
		line(cx, cy, circle_segs + 1, 0.5, 0.25, 0.0);
	}
}

// world — simulation container implementation

#include <cmath>
#include <cstdlib>
#include <ctime>
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

	// Phase 4: initial positions and orientations for Attacker and Defender
	Attacker.X = 15;
	Attacker.Y = 15;
	Attacker.theta_chassis = 0.25 * M_PI;
	Attacker.theta_laser = 0.25 * M_PI;
	Attacker.laserOn = true;   // attack mode: draw laser
	Defender.X = 57;
	Defender.Y = 57;
	Defender.theta_chassis = -0.75 * M_PI;
	Defender.theta_laser = -0.75 * M_PI;
	Defender.laserOn = false;   // defense mode: laser off, do not draw
	los_clear = true;
	// Use same shuffle logic so initial layout has no robot–obstacle overlap
	Shuffle();
}

void world::Shuffle() {
	// Reposition all active obstacles; avoid obstacle–obstacle overlap
	for (int i = 0; i < N_OBSTACLES_MAX; i++) {
		if (!Obstacles[i].isActive) continue;
		double R = Obstacles[i].R;
		const double margin = R;
		const int max_tries = 50;
		for (int t = 0; t < max_tries; t++) {
			double nx = margin + (double)rand() / (double)RAND_MAX * (WorldWidthInches - 2.0 * margin);
			double ny = margin + (double)rand() / (double)RAND_MAX * (WorldHeightInches - 2.0 * margin);
			bool ok = true;
			for (int j = 0; j < N_OBSTACLES_MAX; j++) {
				if (j == i || !Obstacles[j].isActive) continue;
				double dx = nx - Obstacles[j].X, dy = ny - Obstacles[j].Y;
				if (dx * dx + dy * dy < (R + Obstacles[j].R) * (R + Obstacles[j].R)) {
					ok = false;
					break;
				}
			}
			if (ok) {
				Obstacles[i].X = nx;
				Obstacles[i].Y = ny;
				break;
			}
		}
	}
	// Reposition Attacker and Defender; avoid spawning inside obstacles
	const double margin = RobotRadiusInches;
	const int max_tries = 80;
	for (int t = 0; t < max_tries; t++) {
		double ax = margin + (double)rand() / (double)RAND_MAX * (WorldWidthInches - 2.0 * margin);
		double ay = margin + (double)rand() / (double)RAND_MAX * (WorldHeightInches - 2.0 * margin);
		double dx = margin + (double)rand() / (double)RAND_MAX * (WorldWidthInches - 2.0 * margin);
		double dy = margin + (double)rand() / (double)RAND_MAX * (WorldHeightInches - 2.0 * margin);
		bool a_ok = true, d_ok = true;
		for (int i = 0; i < N_OBSTACLES_MAX; i++) {
			if (!Obstacles[i].isActive) continue;
			double oR = Obstacles[i].R;
			double minDist = oR + RobotRadiusInches;
			double a_dx = ax - Obstacles[i].X, a_dy = ay - Obstacles[i].Y;
			if (a_dx * a_dx + a_dy * a_dy < minDist * minDist) a_ok = false;
			double d_dx = dx - Obstacles[i].X, d_dy = dy - Obstacles[i].Y;
			if (d_dx * d_dx + d_dy * d_dy < minDist * minDist) d_ok = false;
		}
		// Also avoid placing Attacker and Defender on top of each other
		double ad_dx = ax - dx, ad_dy = ay - dy;
		if (ad_dx * ad_dx + ad_dy * ad_dy < (2.0 * RobotRadiusInches) * (2.0 * RobotRadiusInches))
			a_ok = d_ok = false;
		if (a_ok && d_ok) {
			Attacker.X = ax;
			Attacker.Y = ay;
			Defender.X = dx;
			Defender.Y = dy;
			Attacker.theta_chassis = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
			Attacker.theta_laser = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
			Defender.theta_chassis = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
			Defender.theta_laser = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
			break;
		}
	}
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
	(void)dt;
	// Aim Attacker's turret at Defender so the yellow line = "laser aim" matches LoS direction
	Attacker.theta_laser = atan2(Defender.Y - Attacker.Y, Defender.X - Attacker.X);
	// Phase 6: ray–circle LoS from Attacker to Defender
	double ax = Attacker.X, ay = Attacker.Y;
	double dx = Defender.X, dy = Defender.Y;
	double abx = dx - ax, aby = dy - ay;
	double dot_AB = abx * abx + aby * aby;
	const double eps = 1e-6;
	los_clear = true;
	if (dot_AB < eps) {
		// Segment degenerate (same point); no obstacle can block
		return;
	}
	for (int i = 0; i < N_OBSTACLES_MAX; i++) {
		if (!Obstacles[i].isActive) continue;
		double ox = Obstacles[i].X, oy = Obstacles[i].Y, R = Obstacles[i].R;
		double acx = ox - ax, acy = oy - ay;
		double t = (acx * abx + acy * aby) / dot_AB;
		if (t < 0.0) t = 0.0;
		if (t > 1.0) t = 1.0;
		double qx = ax + t * abx, qy = ay + t * aby;
		double dqx = ox - qx, dqy = oy - qy;
		if (dqx * dqx + dqy * dqy < R * R) {
			los_clear = false;
			return;
		}
	}
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

	// Phase 4: draw robots (robot.cpp); Attacker has laser on, Defender does not
	::Draw(Attacker, *this, true);
	::Draw(Defender, *this, false);

	// Phase 6: LoS line Attacker → Defender (green = clear, red = blocked)
	double lx[2], ly[2];
	CameraToScreen(Attacker.X, Attacker.Y, lx[0], ly[0]);
	CameraToScreen(Defender.X, Defender.Y, lx[1], ly[1]);
	if (los_clear)
		line(lx, ly, 2, 0.0, 0.8, 0.0);
	else
		line(lx, ly, 2, 0.9, 0.0, 0.0);
}

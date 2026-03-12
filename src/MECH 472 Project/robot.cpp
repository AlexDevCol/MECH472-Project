// robot — draw implementation

#include <cmath>
#include "robot.h"
#include "world.h"
#include "2D_graphics.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Draw(robot const& r, world const& w, bool isAttacker) {
	const double half_len = 0.5 * RobotBodyLengthInches;
	const double half_wid = 0.5 * RobotBodyWidthInches;
	double cx = r.X, cy = r.Y;
	double th = r.theta_chassis;
	double c = cos(th), s = sin(th);
	// Four corners of body rectangle in camera-space inches
	double dx0 = half_len * c - half_wid * s, dy0 = half_len * s + half_wid * c;
	double dx1 = half_len * c + half_wid * s, dy1 = half_len * s - half_wid * c;
	double wx[4], wy[4];
	wx[0] = cx + dx0; wy[0] = cy + dy0;
	wx[1] = cx + dx1; wy[1] = cy + dy1;
	wx[2] = cx - dx0; wy[2] = cy - dy0;
	wx[3] = cx - dx1; wy[3] = cy - dy1;
	double sx[4], sy[4];
	for (int i = 0; i < 4; i++)
		w.CameraToScreen(wx[i], wy[i], sx[i], sy[i]);
	// Body as two triangles; Attacker = blue, Defender = green
	double bodyR = isAttacker ? 0.2 : 0.2, bodyG = isAttacker ? 0.3 : 0.6, bodyB = isAttacker ? 0.8 : 0.2;
	double x3[3], y3[3], R3[3], G3[3], B3[3];
	for (int i = 0; i < 3; i++) { R3[i] = bodyR; G3[i] = bodyG; B3[i] = bodyB; }
	x3[0] = sx[0]; y3[0] = sy[0]; x3[1] = sx[1]; y3[1] = sy[1]; x3[2] = sx[2]; y3[2] = sy[2];
	triangle(x3, y3, R3, G3, B3);
	x3[0] = sx[0]; y3[0] = sy[0]; x3[1] = sx[2]; y3[1] = sy[2]; x3[2] = sx[3]; y3[2] = sy[3];
	triangle(x3, y3, R3, G3, B3);

	// Turret circle (at most 1/3 body width)
	const int turret_segs = 24;
	double tr = TurretCircleRadiusInches;
	double tx[25], ty[25];
	for (int i = 0; i <= turret_segs; i++) {
		double a = (double)i / (double)turret_segs * 2.0 * M_PI;
		double px = cx + tr * cos(a), py = cy + tr * sin(a);
		w.CameraToScreen(px, py, tx[i], ty[i]);
	}
	line(tx, ty, turret_segs + 1, 0.9, 0.9, 0.9);

	// Laser line only when laser is on (attack mode); extends way beyond body
	// Use white/amber so it's distinct from the LoS line (green = clear, red = blocked)
	if (r.laserOn) {
		double len = LaserLineLengthInches;
		double lx[2], ly[2];
		w.CameraToScreen(cx, cy, lx[0], ly[0]);
		w.CameraToScreen(cx + len * cos(r.theta_laser), cy + len * sin(r.theta_laser), lx[1], ly[1]);
		line(lx, ly, 2, 0.95, 0.95, 0.6);
	}
}

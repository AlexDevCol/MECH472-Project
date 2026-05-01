// robot — draw implementation

#include <cmath>
#include "robot.h"
#include "world.h"
#include "3D_graphics.h"
#include "graphics.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Sprite IDs loaded in draw_3D_graphics.cpp via create_sprite_2D
static const int kSpriteAttacker = 0;
static const int kSpriteDefender = 1;

// CameraToScreen outputs pixels (see world.cpp). BMP texels → pixel scale for hull width.
extern int WINDOW_WIDTH;

static const double kRobotBmpWidthPx = 168.0;
// BMP forward axis vs chassis theta (+X forward); adjust visually if needed.
static const double kRobotSpriteThetaOffset = M_PI / 2.0;

// Local buffers only — avoid aliasing globals while draw_line / draw_triangle_list runs.
static void line(double* x, double* y, int n, double R, double G, double B) {
	if (n <= 0) return;
	const int MAXN = 512;
	if (n > MAXN) n = MAXN;
	double lx[MAXN], ly[MAXN], lz[MAXN], lr[MAXN], lg[MAXN], lb[MAXN];
	for (int i = 0; i < n; i++) {
		lx[i] = x[i];
		ly[i] = y[i];
		lz[i] = 0.0;
		lr[i] = R;
		lg[i] = G;
		lb[i] = B;
	}
	draw_line(lx, ly, lz, lr, lg, lb, n);
}

void Draw(robot const& r, world const& w, bool isAttacker) {
	double cx = r.X, cy = r.Y;
	double th = r.theta_chassis;

	double sx, sy;
	w.CameraToScreen(cx, cy, sx, sy);

	int sprite_id = isAttacker ? kSpriteAttacker : kSpriteDefender;
	const double pxPerInch = (double)WINDOW_WIDTH / WorldWidthInches;
	const double spriteScale =
		(RobotBodyLengthInches * pxPerInch) / kRobotBmpWidthPx;
	draw_sprite_2D(sprite_id, sx, sy, th + kRobotSpriteThetaOffset, spriteScale);

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

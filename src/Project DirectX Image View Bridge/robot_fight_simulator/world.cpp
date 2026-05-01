// world — simulation container implementation

#include <cmath>
#include <cstdlib>
#include <ctime>
#include "world.h"
#include "global_data.h"
#include "3D_graphics.h"
#include "graphics.h"

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 2D-compat wrappers — local buffers only (avoid aliasing draw_line's internal XB writes).
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

world::world() {
	N_obstacles = 5;
	// Fixed initial positions (inches) and radii
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

	//initial positions and orientations for Attacker and Defender
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
	blocking_obstacle_index = -1;
	attacker_contour_side = -1;
	attacker_contour_obs = -1;
	attacker_wp_x = attacker_wp_y = 0.0;
	attacker_stall_frames = 0;
	attacker_prev_dist_sq = 1e30;
	defender_stall_frames = 0;
	defender_prev_dist_to_target_sq = 1e30;

	// Use same shuffle logic so initial layout has no robot–obstacle overlap
	Shuffle();
}

void world::Shuffle() {
	// Visualize logic: robots on opposite corners, one obstacle directly between them (LoS blocked for Defender)
	const double margin = RobotRadiusInches;
	// Attacker at one corner (top-right in camera space)
	Attacker.X = margin;
	Attacker.Y = margin;
	// Defender at opposite corner (bottom-left)
	Defender.X = WorldWidthInches - margin;
	Defender.Y = WorldHeightInches - margin;
	// One obstacle directly between them (midpoint of segment) so Defender has cover
	Obstacles[0].X = (Attacker.X + Defender.X) * 0.5;
	Obstacles[0].Y = (Attacker.Y + Defender.Y) * 0.5;
	Obstacles[0].R = 5.0;
	Obstacles[0].isActive = true;
	// Remaining obstacles: random positions, avoid obstacle 0 and both robots
	for (int i = 1; i < N_OBSTACLES_MAX; i++) {
		if (!Obstacles[i].isActive) continue;
		double R = Obstacles[i].R;
		const double obs_margin = R;
		const int max_tries = 50;
		for (int t = 0; t < max_tries; t++) {
			double nx = obs_margin + (double)rand() / (double)RAND_MAX * (WorldWidthInches - 2.0 * obs_margin);
			double ny = obs_margin + (double)rand() / (double)RAND_MAX * (WorldHeightInches - 2.0 * obs_margin);
			bool ok = true;
			// Avoid obstacle 0 (the one between robots)
			double d0x = nx - Obstacles[0].X, d0y = ny - Obstacles[0].Y;
			if (d0x * d0x + d0y * d0y < (R + Obstacles[0].R) * (R + Obstacles[0].R)) ok = false;
			// Avoid Attacker and Defender
			double dax = nx - Attacker.X, day = ny - Attacker.Y;
			if (dax * dax + day * day < (R + RobotRadiusInches) * (R + RobotRadiusInches)) ok = false;
			double ddx = nx - Defender.X, ddy = ny - Defender.Y;
			if (ddx * ddx + ddy * ddy < (R + RobotRadiusInches) * (R + RobotRadiusInches)) ok = false;
			for (int j = 1; j < N_OBSTACLES_MAX; j++) {
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
	// Orientations and targets
	Attacker.theta_chassis = atan2(Defender.Y - Attacker.Y, Defender.X - Attacker.X);
	Attacker.theta_laser = Attacker.theta_chassis;
	Attacker.target_x = Defender.X;
	Attacker.target_y = Defender.Y;
	Defender.theta_chassis = atan2(Attacker.Y - Defender.Y, Attacker.X - Defender.X);
	Defender.theta_laser = Defender.theta_chassis;
	Defender.target_x = Obstacles[0].X;
	Defender.target_y = Obstacles[0].Y;
	// Reset Phase 10 contour state on shuffle
	attacker_contour_side = -1;
	attacker_contour_obs = -1;
	defender_stall_frames = 0;
	defender_prev_dist_to_target_sq = 1e30;
}

world::~world() {
	// Phase 3: fixed array, nothing to delete
}

// Phase 10: OBB (robot rectangle) vs circle (obstacle) overlap test
static bool OBBCircleOverlap(double cx, double cy, double theta,
	double halfL, double halfW,
	double ox, double oy, double R) {
	double dx = ox - cx, dy = oy - cy;
	double c = cos(-theta), s = sin(-theta);
	double lx = dx * c - dy * s;
	double ly = dx * s + dy * c;
	double clampX = lx < -halfL ? -halfL : (lx > halfL ? halfL : lx);
	double clampY = ly < -halfW ? -halfW : (ly > halfW ? halfW : ly);
	double ex = lx - clampX, ey = ly - clampY;
	return (ex * ex + ey * ey) < R * R;
}

void world::CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const {
	// WIDTH_2D / HEIGHT_2D match the framebuffer (pixels); convert simulation inches to pixels.
	out_x = x_inches * ((double)WINDOW_WIDTH / WorldWidthInches);
	out_y = y_inches * ((double)WINDOW_HEIGHT / WorldHeightInches);
}

void world::Update(double dt) {
	(void)dt;
	// Aim Attacker's turret at Defender; clamp to 0..180 deg (90 = forward) relative to chassis
	double desired_world = atan2(Defender.Y - Attacker.Y, Defender.X - Attacker.X);
	double rel_rad = desired_world - Attacker.theta_chassis;
	while (rel_rad > M_PI) rel_rad -= 2.0 * M_PI;
	while (rel_rad < -M_PI) rel_rad += 2.0 * M_PI;
	double rel_deg = rel_rad * (180.0 / M_PI);
	if (rel_deg < -TurretHalfRangeDeg) rel_deg = -TurretHalfRangeDeg;
	if (rel_deg > TurretHalfRangeDeg) rel_deg = TurretHalfRangeDeg;
	Attacker.theta_laser = Attacker.theta_chassis + rel_deg * (M_PI / 180.0);
	// Phase 6: ray–circle LoS from Attacker to Defender
	double ax = Attacker.X, ay = Attacker.Y;
	double dx = Defender.X, dy = Defender.Y;
	double abx = dx - ax, aby = dy - ay;
	double dot_AB = abx * abx + aby * aby;
	const double eps = 1e-6;
	los_clear = true;
	blocking_obstacle_index = -1;
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
			blocking_obstacle_index = i;
			break;
		}
	}
	// Phase 7 + 10: Attacker target — committed contour (sticky waypoint) to avoid free-spinning
	double dist_to_defender_sq = (ax - dx) * (ax - dx) + (ay - dy) * (ay - dy);
	if (los_clear) {
		Attacker.target_x = Defender.X;
		Attacker.target_y = Defender.Y;
		attacker_contour_side = -1;
		attacker_contour_obs = -1;
		attacker_stall_frames = 0;
	} else if (blocking_obstacle_index >= 0 && Obstacles[blocking_obstacle_index].isActive) {
		int bi = blocking_obstacle_index;
		double ox = Obstacles[bi].X, oy = Obstacles[bi].Y, R = Obstacles[bi].R;
		double vx = ox - ax, vy = oy - ay;
		double len = sqrt(vx * vx + vy * vy);
		if (len >= eps) {
			vx /= len;
			vy /= len;
			double clearance = R + RobotRadiusInches + WaypointClearanceInches;
			double perp0_x = -vy, perp0_y = vx;   // "left"
			double perp1_x = vy, perp1_y = -vx;   // "right"
			double wp0_x = ox + clearance * perp0_x, wp0_y = oy + clearance * perp0_y;
			double wp1_x = ox + clearance * perp1_x, wp1_y = oy + clearance * perp1_y;
			double d0_sq = (dx - wp0_x) * (dx - wp0_x) + (dy - wp0_y) * (dy - wp0_y);
			double d1_sq = (dx - wp1_x) * (dx - wp1_x) + (dy - wp1_y) * (dy - wp1_y);
			int preferred_side = (d0_sq <= d1_sq) ? 0 : 1;
			// Phase 10: gap feasibility — can robot fit through passage on this side?
			auto passage_ok = [this, bi](double wx, double wy) {
				if (wx < MinPassageWidthInches || wx > WorldWidthInches - MinPassageWidthInches) return false;
				if (wy < MinPassageWidthInches || wy > WorldHeightInches - MinPassageWidthInches) return false;
				for (int j = 0; j < N_OBSTACLES_MAX; j++) {
					if (j == bi || !Obstacles[j].isActive) continue;
					double ddx = wx - Obstacles[j].X, ddy = wy - Obstacles[j].Y;
					double min_dist = Obstacles[j].R + MinPassageWidthInches;
					if (ddx * ddx + ddy * ddy < min_dist * min_dist) return false;
				}
				return true;
			};
			bool gap_ok_0 = passage_ok(wp0_x, wp0_y);
			bool gap_ok_1 = passage_ok(wp1_x, wp1_y);
			int chosen_side = preferred_side;
			if (preferred_side == 0 && !gap_ok_0 && gap_ok_1) chosen_side = 1;
			else if (preferred_side == 1 && !gap_ok_1 && gap_ok_0) chosen_side = 0;
			bool need_new_waypoint = false;
			if (attacker_contour_side < 0 || attacker_contour_obs != bi) {
				need_new_waypoint = true;
				attacker_contour_obs = bi;
				attacker_contour_side = chosen_side;
				attacker_stall_frames = 0;
				attacker_prev_dist_sq = dist_to_defender_sq;
			} else {
				double wp_x = (attacker_contour_side == 0) ? wp0_x : wp1_x;
				double wp_y = (attacker_contour_side == 0) ? wp0_y : wp1_y;
				double to_wp_sq = (ax - wp_x) * (ax - wp_x) + (ay - wp_y) * (ay - wp_y);
				if (to_wp_sq < ContourArrivalRadius * ContourArrivalRadius) {
					need_new_waypoint = true;
					attacker_contour_side = -1;
					attacker_contour_obs = -1;
				} else if (dist_to_defender_sq < attacker_prev_dist_sq - ContourProgressEpsilon) {
					attacker_stall_frames = 0;
					attacker_prev_dist_sq = dist_to_defender_sq;
				} else {
					attacker_stall_frames++;
					if (attacker_stall_frames >= ContourStallThreshold) {
						need_new_waypoint = true;
						attacker_contour_side = 1 - attacker_contour_side;
						attacker_stall_frames = 0;
						attacker_prev_dist_sq = dist_to_defender_sq;
					}
				}
			}
			if (need_new_waypoint && attacker_contour_side < 0) {
				Attacker.target_x = Defender.X;
				Attacker.target_y = Defender.Y;
			} else if (need_new_waypoint && attacker_contour_side >= 0) {
				attacker_wp_x = (attacker_contour_side == 0) ? wp0_x : wp1_x;
				attacker_wp_y = (attacker_contour_side == 0) ? wp0_y : wp1_y;
				Attacker.target_x = attacker_wp_x;
				Attacker.target_y = attacker_wp_y;
			} else {
				Attacker.target_x = attacker_wp_x;
				Attacker.target_y = attacker_wp_y;
			}
		} else {
			Attacker.target_x = Defender.X;
			Attacker.target_y = Defender.Y;
			attacker_contour_side = -1;
			attacker_contour_obs = -1;
		}
	} else {
		Attacker.target_x = Defender.X;
		Attacker.target_y = Defender.Y;
		attacker_contour_side = -1;
		attacker_contour_obs = -1;
	}

	// Phase 8: Defender target = shadow point behind an obstacle (from Attacker)
	double best_sx = Defender.X, best_sy = Defender.Y;
	double best_dist_sq = 1e30;
	bool have_valid = false;
	// Option B: prefer blocking obstacle's shadow if valid
	if (blocking_obstacle_index >= 0 && Obstacles[blocking_obstacle_index].isActive) {
		int i = blocking_obstacle_index;
		double ox = Obstacles[i].X, oy = Obstacles[i].Y, R = Obstacles[i].R;
		double vx = ox - ax, vy = oy - ay;
		double len = sqrt(vx * vx + vy * vy);
		if (len >= eps) {
			vx /= len;
			vy /= len;
			double ext = R + RobotRadiusInches + ShadowBufferInches;
			double sx = ox + ext * vx, sy = oy + ext * vy;
			// Valid: in bounds and not wall-adjacent (avoid cornering)
			if (sx >= RobotRadiusInches && sx <= WorldWidthInches - RobotRadiusInches &&
			    sy >= RobotRadiusInches && sy <= WorldHeightInches - RobotRadiusInches) {
				// Reject if shadow lands inside another obstacle
				bool inside_obs = false;
				for (int j = 0; j < N_OBSTACLES_MAX; j++) {
					if (!Obstacles[j].isActive) continue;
					double sdx = sx - Obstacles[j].X, sdy = sy - Obstacles[j].Y;
					double min_dist = Obstacles[j].R + RobotRadiusInches;
					if (sdx * sdx + sdy * sdy < min_dist * min_dist) {
						inside_obs = true;
						break;
					}
				}
				if (!inside_obs) {
					// Phase 10: blind-spot bonus — prefer shadow behind attacker's turret
					double to_shadow = atan2(sy - Attacker.Y, sx - Attacker.X);
					double rel = to_shadow - Attacker.theta_chassis;
					while (rel > M_PI) rel -= 2.0 * M_PI;
					while (rel < -M_PI) rel += 2.0 * M_PI;
					double blind = (fabs(rel) > TurretHalfRangeDeg * (M_PI / 180.0)) ? BlindSpotBonus : 1.0;
					double ddx = sx - Defender.X, ddy = sy - Defender.Y;
					double dist_sq = ddx * ddx + ddy * ddy;
					double effective_sq = dist_sq * blind;
					if (effective_sq < best_dist_sq) {
						best_dist_sq = effective_sq;
						best_sx = sx;
						best_sy = sy;
						have_valid = true;
					}
				}
			}
		}
	}
	if (!have_valid) {
		for (int i = 0; i < N_OBSTACLES_MAX; i++) {
			if (!Obstacles[i].isActive) continue;
			double ox = Obstacles[i].X, oy = Obstacles[i].Y, R = Obstacles[i].R;
			double vx = ox - ax, vy = oy - ay;
			double len = sqrt(vx * vx + vy * vy);
			if (len < eps) continue;
			vx /= len;
			vy /= len;
			double ext = R + RobotRadiusInches + ShadowBufferInches;
			double sx = ox + ext * vx, sy = oy + ext * vy;
			if (sx < RobotRadiusInches || sx > WorldWidthInches - RobotRadiusInches ||
			    sy < RobotRadiusInches || sy > WorldHeightInches - RobotRadiusInches) continue;
			// Reject if shadow lands inside another obstacle
			bool inside_obs = false;
			for (int j = 0; j < N_OBSTACLES_MAX; j++) {
				if (!Obstacles[j].isActive) continue;
				double sdx = sx - Obstacles[j].X, sdy = sy - Obstacles[j].Y;
				double min_dist = Obstacles[j].R + RobotRadiusInches;
				if (sdx * sdx + sdy * sdy < min_dist * min_dist) {
					inside_obs = true;
					break;
				}
			}
			if (inside_obs) continue;
			double ddx = sx - Defender.X, ddy = sy - Defender.Y;
			double dist_sq = ddx * ddx + ddy * ddy;
			// Phase 10: blind-spot bonus
			double to_shadow = atan2(sy - Attacker.Y, sx - Attacker.X);
			double rel = to_shadow - Attacker.theta_chassis;
			while (rel > M_PI) rel -= 2.0 * M_PI;
			while (rel < -M_PI) rel += 2.0 * M_PI;
			double blind = (fabs(rel) > TurretHalfRangeDeg * (M_PI / 180.0)) ? BlindSpotBonus : 1.0;
			double effective_sq = dist_sq * blind;
			// Phase 10: when stalled, penalize current target so we try another shadow
			if (defender_stall_frames >= DefenderStallThreshold) {
				double to_cur_sq = (sx - Defender.target_x) * (sx - Defender.target_x) + (sy - Defender.target_y) * (sy - Defender.target_y);
				if (to_cur_sq < ShadowArrivalInches * ShadowArrivalInches * 4.0)
					effective_sq *= 10.0;
			}
			if (effective_sq < best_dist_sq) {
				best_dist_sq = effective_sq;
				best_sx = sx;
				best_sy = sy;
				have_valid = true;
			}
		}
	}
	if (!have_valid) {
		best_sx = Defender.X;
		best_sy = Defender.Y;
	}
	Defender.target_x = best_sx;
	Defender.target_y = best_sy;
	// Hold when safe: LoS blocked and already close to shadow target — stop chasing to avoid cornering
	if (!los_clear) {
		double d2 = (Defender.X - best_sx) * (Defender.X - best_sx) + (Defender.Y - best_sy) * (Defender.Y - best_sy);
		if (d2 < ShadowArrivalInches * ShadowArrivalInches) {
			Defender.target_x = Defender.X;
			Defender.target_y = Defender.Y;
		}
	}
	// Phase 10: defender stall detection — progress toward current target
	double dist_to_target_sq = (Defender.X - Defender.target_x) * (Defender.X - Defender.target_x) + (Defender.Y - Defender.target_y) * (Defender.Y - Defender.target_y);
	if (dist_to_target_sq < defender_prev_dist_to_target_sq - DefenderShadowProgressEpsilon)
		defender_stall_frames = 0;
	else
		defender_stall_frames++;
	defender_prev_dist_to_target_sq = dist_to_target_sq;

	// Phase 9: APF pathfinding — non-holonomic: turn toward desired direction, drive forward along chassis
	const double min_d = 0.5;  // avoid div by zero in repulsion
	const double min_force = 1e-3;  // skip move when total force negligible (avoid jitter)
	// Attacker: APF toward Defender (target already set)
	{
		double rx = Attacker.X, ry = Attacker.Y;
		double tx = Attacker.target_x, ty = Attacker.target_y;
		double fax = tx - rx, fay = ty - ry;
		double dist_att = sqrt(fax * fax + fay * fay);
		if (dist_att > eps) {
			fax /= dist_att;
			fay /= dist_att;
		} else {
			fax = fay = 0.0;
		}
		double fx = fax, fy = fay;
		for (int i = 0; i < N_OBSTACLES_MAX; i++) {
			if (!Obstacles[i].isActive) continue;
			double ox = Obstacles[i].X, oy = Obstacles[i].Y, R = Obstacles[i].R;
			double d_safe = R + APFRadiusInches + APFSafetyMarginInches;
			double dx = rx - ox, dy = ry - oy;
			double d = sqrt(dx * dx + dy * dy);
			if (d < min_d) d = min_d;
			if (d < d_safe) {
				double mag = APFRepulsiveGain * (1.0 / d - 1.0 / d_safe);
				if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
				double nx = dx / d, ny = dy / d;
				fx += mag * nx;
				fy += mag * ny;
				// Tangential component to orbit around obstacle (break deadlock)
				double dot_t = -ny * fay + nx * fax;
				double tx_t = (dot_t > 0.0) ? -ny : ny;
				double ty_t = (dot_t > 0.0) ? nx : -nx;
				fx += APFTangentialFraction * mag * tx_t;
				fy += APFTangentialFraction * mag * ty_t;
			}
		}
		// Robot-robot repulsion: Attacker repelled by Defender
		double adx = rx - Defender.X, ady = ry - Defender.Y;
		double d_robot = sqrt(adx * adx + ady * ady);
		if (d_robot < min_d) d_robot = min_d;
		if (d_robot < RobotRobotRepelDistInches) {
			double mag = APFRepulsiveGain * (1.0 / d_robot - 1.0 / RobotRobotRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx += mag * (adx / d_robot);
			fy += mag * (ady / d_robot);
		}
		// Wall repulsion
		double wx = rx; if (wx < min_d) wx = min_d;
		if (wx < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wx - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx += mag;
		}
		double wx2 = WorldWidthInches - rx; if (wx2 < min_d) wx2 = min_d;
		if (wx2 < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wx2 - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx -= mag;
		}
		double wy = ry; if (wy < min_d) wy = min_d;
		if (wy < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wy - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fy += mag;
		}
		double wy2 = WorldHeightInches - ry; if (wy2 < min_d) wy2 = min_d;
		if (wy2 < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wy2 - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fy -= mag;
		}
		double f_len = sqrt(fx * fx + fy * fy);
		if (f_len > min_force) {
			double desired_theta = atan2(fy, fx);
			// Turn chassis toward desired direction
			double diff = desired_theta - Attacker.theta_chassis;
			while (diff > M_PI) diff -= 2.0 * M_PI;
			while (diff < -M_PI) diff += 2.0 * M_PI;
			double step = diff;
			if (step > ChassisTurnRateRadPerFrame) step = ChassisTurnRateRadPerFrame;
			if (step < -ChassisTurnRateRadPerFrame) step = -ChassisTurnRateRadPerFrame;
			Attacker.theta_chassis += step;
			while (Attacker.theta_chassis > M_PI) Attacker.theta_chassis -= 2.0 * M_PI;
			while (Attacker.theta_chassis < -M_PI) Attacker.theta_chassis += 2.0 * M_PI;
			// Move forward along chassis (non-holonomic): speed scaled by alignment
			double angle_diff = desired_theta - Attacker.theta_chassis;
			while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
			while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
			double cos_a = cos(angle_diff);
			if (cos_a < 0.0) cos_a = 0.0;
			double forward_speed = RobotSpeedInchesPerFrame * cos_a;
			double new_ax = rx + forward_speed * cos(Attacker.theta_chassis);
			double new_ay = ry + forward_speed * sin(Attacker.theta_chassis);
			bool would_penetrate = false;
			for (int i = 0; i < N_OBSTACLES_MAX; i++) {
				if (!Obstacles[i].isActive) continue;
				if (OBBCircleOverlap(new_ax, new_ay, Attacker.theta_chassis,
					0.5 * RobotBodyLengthInches, 0.5 * RobotBodyWidthInches,
					Obstacles[i].X, Obstacles[i].Y, Obstacles[i].R)) {
					would_penetrate = true;
					break;
				}
			}
			if (!would_penetrate && OBBCircleOverlap(new_ax, new_ay, Attacker.theta_chassis,
				0.5 * RobotBodyLengthInches, 0.5 * RobotBodyWidthInches,
				Defender.X, Defender.Y, RobotRadiusInches))
				would_penetrate = true;
			if (!would_penetrate) {
				Attacker.X = new_ax;
				Attacker.Y = new_ay;
			}
		}
		if (Attacker.X < 0.0) Attacker.X = 0.0;
		if (Attacker.X > WorldWidthInches) Attacker.X = WorldWidthInches;
		if (Attacker.Y < 0.0) Attacker.Y = 0.0;
		if (Attacker.Y > WorldHeightInches) Attacker.Y = WorldHeightInches;
	}
	// Defender: APF toward shadow target (same non-holonomic model)
	{
		double rx = Defender.X, ry = Defender.Y;
		double tx = Defender.target_x, ty = Defender.target_y;
		double fax = tx - rx, fay = ty - ry;
		double dist_att = sqrt(fax * fax + fay * fay);
		if (dist_att > eps) {
			fax /= dist_att;
			fay /= dist_att;
		} else {
			fax = fay = 0.0;
		}
		double fx = fax, fy = fay;
		for (int i = 0; i < N_OBSTACLES_MAX; i++) {
			if (!Obstacles[i].isActive) continue;
			double ox = Obstacles[i].X, oy = Obstacles[i].Y, R = Obstacles[i].R;
			double d_safe = R + APFRadiusInches + APFSafetyMarginInches;
			double dx = rx - ox, dy = ry - oy;
			double d = sqrt(dx * dx + dy * dy);
			if (d < min_d) d = min_d;
			if (d < d_safe) {
				double mag = APFRepulsiveGain * (1.0 / d - 1.0 / d_safe);
				if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
				double nx = dx / d, ny = dy / d;
				fx += mag * nx;
				fy += mag * ny;
				double dot_t = -ny * fay + nx * fax;
				double tx_t = (dot_t > 0.0) ? -ny : ny;
				double ty_t = (dot_t > 0.0) ? nx : -nx;
				fx += APFTangentialFraction * mag * tx_t;
				fy += APFTangentialFraction * mag * ty_t;
			}
		}
		// Robot-robot repulsion: Defender repelled by Attacker
		double dax = rx - Attacker.X, day = ry - Attacker.Y;
		double d_robot = sqrt(dax * dax + day * day);
		if (d_robot < min_d) d_robot = min_d;
		if (d_robot < RobotRobotRepelDistInches) {
			double mag = APFRepulsiveGain * (1.0 / d_robot - 1.0 / RobotRobotRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx += mag * (dax / d_robot);
			fy += mag * (day / d_robot);
		}
		// Wall repulsion
		double wx = rx; if (wx < min_d) wx = min_d;
		if (wx < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wx - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx += mag;
		}
		double wx2 = WorldWidthInches - rx; if (wx2 < min_d) wx2 = min_d;
		if (wx2 < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wx2 - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fx -= mag;
		}
		double wy = ry; if (wy < min_d) wy = min_d;
		if (wy < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wy - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fy += mag;
		}
		double wy2 = WorldHeightInches - ry; if (wy2 < min_d) wy2 = min_d;
		if (wy2 < WallRepelDistInches) {
			double mag = WallRepelGain * (1.0 / wy2 - 1.0 / WallRepelDistInches);
			if (mag > MaxRepulsiveForce) mag = MaxRepulsiveForce;
			fy -= mag;
		}
		double f_len = sqrt(fx * fx + fy * fy);
		if (f_len > min_force) {
			double desired_theta = atan2(fy, fx);
			double diff = desired_theta - Defender.theta_chassis;
			while (diff > M_PI) diff -= 2.0 * M_PI;
			while (diff < -M_PI) diff += 2.0 * M_PI;
			double step = diff;
			if (step > ChassisTurnRateRadPerFrame) step = ChassisTurnRateRadPerFrame;
			if (step < -ChassisTurnRateRadPerFrame) step = -ChassisTurnRateRadPerFrame;
			Defender.theta_chassis += step;
			while (Defender.theta_chassis > M_PI) Defender.theta_chassis -= 2.0 * M_PI;
			while (Defender.theta_chassis < -M_PI) Defender.theta_chassis += 2.0 * M_PI;
			double angle_diff = desired_theta - Defender.theta_chassis;
			while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
			while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
			double cos_a = cos(angle_diff);
			if (cos_a < 0.0) cos_a = 0.0;
			double forward_speed = RobotSpeedInchesPerFrame * cos_a;
			double new_dx = rx + forward_speed * cos(Defender.theta_chassis);
			double new_dy = ry + forward_speed * sin(Defender.theta_chassis);
			bool would_penetrate = false;
			for (int i = 0; i < N_OBSTACLES_MAX; i++) {
				if (!Obstacles[i].isActive) continue;
				if (OBBCircleOverlap(new_dx, new_dy, Defender.theta_chassis,
					0.5 * RobotBodyLengthInches, 0.5 * RobotBodyWidthInches,
					Obstacles[i].X, Obstacles[i].Y, Obstacles[i].R)) {
					would_penetrate = true;
					break;
				}
			}
			if (!would_penetrate && OBBCircleOverlap(new_dx, new_dy, Defender.theta_chassis,
				0.5 * RobotBodyLengthInches, 0.5 * RobotBodyWidthInches,
				Attacker.X, Attacker.Y, RobotRadiusInches))
				would_penetrate = true;
			if (!would_penetrate) {
				Defender.X = new_dx;
				Defender.Y = new_dy;
			}
		}
		if (Defender.X < 0.0) Defender.X = 0.0;
		if (Defender.X > WorldWidthInches) Defender.X = WorldWidthInches;
		if (Defender.Y < 0.0) Defender.Y = 0.0;
		if (Defender.Y > WorldHeightInches) Defender.Y = WorldHeightInches;
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

	// Phase 8: debug marker at Defender's shadow target (cyan)
	const int marker_segs = 16;
	const double marker_radius_inches = 2.0;
	double mx[marker_segs + 1], my[marker_segs + 1];
	double tx = Defender.target_x, ty = Defender.target_y;
	for (int s = 0; s <= marker_segs; s++) {
		double a = (double)s / (double)marker_segs * 2.0 * M_PI;
		double px = tx + marker_radius_inches * cos(a);
		double py = ty + marker_radius_inches * sin(a);
		CameraToScreen(px, py, mx[s], my[s]);
	}
	line(mx, my, marker_segs + 1, 0.0, 0.8, 0.8);
}

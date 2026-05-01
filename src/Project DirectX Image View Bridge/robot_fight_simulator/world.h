// world — simulation container (single world pattern)

#include "global_data.h"
#include "robot.h"

struct obstacle {
	double X;        // center, inches (camera space)
	double Y;
	double R;       // radius, inches
	bool isActive;
};

class world {
public:
	world();
	~world();

	void Update(double dt);
	void Draw();
	void Shuffle();

	// Inches → framebuffer pixels (matches WIDTH_2D / HEIGHT_2D in draw_3D_graphics.cpp).
	void CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const;

private:
	obstacle Obstacles[N_OBSTACLES_MAX];
	int N_obstacles;  // number of obstacles to use (initialized active)
	robot Attacker;
	robot Defender;
	bool los_clear;   // Phase 6: true if no obstacle blocks Attacker–Defender segment
	int blocking_obstacle_index;  // Phase 8: index of obstacle blocking LoS, or -1 if clear
	// Phase 10: committed contour (attacker)
	int attacker_contour_side;    // -1 = none, 0 = left, 1 = right
	int attacker_contour_obs;      // index of obstacle being contoured
	double attacker_wp_x, attacker_wp_y;  // frozen waypoint
	int attacker_stall_frames;    // frames without progress toward defender
	double attacker_prev_dist_sq;  // previous distance-to-defender squared
	// Phase 10: defender progress toward shadow
	int defender_stall_frames;
	double defender_prev_dist_to_target_sq;
};

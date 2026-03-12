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

	// Used by robot::Draw for camera→screen transform
	void CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const;

private:
	obstacle Obstacles[N_OBSTACLES_MAX];
	int N_obstacles;  // number of obstacles to use (initialized active)
	robot Attacker;
	robot Defender;
	bool los_clear;   // Phase 6: true if no obstacle blocks Attacker–Defender segment
};

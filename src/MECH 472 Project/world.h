// world — simulation container (single world pattern)

#include "global_data.h"

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

private:
	// Convert camera-space (inches, top-right origin) to screen pixels (top-left origin)
	void CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const;

	obstacle Obstacles[N_OBSTACLES_MAX];
	int N_obstacles;  // number of obstacles to use (initialized active)
};

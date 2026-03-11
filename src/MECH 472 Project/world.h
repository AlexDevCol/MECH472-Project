// world — simulation container (single world pattern)

class world {
public:
	world();
	~world();

	void Update(double dt);
	void Draw();

private:
	// Convert camera-space (inches, top-right origin) to screen pixels (top-left origin)
	void CameraToScreen(double x_inches, double y_inches, double& out_x, double& out_y) const;
};

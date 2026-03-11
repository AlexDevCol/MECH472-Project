// world — simulation container (single world pattern)

class world {
public:
	world();
	~world();

	void Update(double dt);
	void Draw();

private:
	// state added in later phases
};

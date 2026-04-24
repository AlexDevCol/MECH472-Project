
// robot simulation class

const int NMAX = 20; // maximum number of state variables

class robot {
	
	public:
	
	int NS; // number of state variables (order)
	int MS; // number of inputs
	int PS; // number of outputs
	
	double t; // current time (seconds)
	
	// state, derivative, input, and output vectors
	double x[NMAX+1];  // state vector x
	double xd[NMAX+1]; // derivative vector at time t
	double u[NMAX+1];
	double y[NMAX+1];
	
	// DC motor model parameters
	double L, R, kb; // electrical model parameters
	double J, km, b, fc; // mechanical model parameters	
	
	// traction model parameters
	double m, Rw, g, Q, GR; ///
	
	// simple car parameters
	double Lc; // length between forward and rear wheel axes (m)
	
	// constructor for initialization
	robot(double x0, double y0, double theta0);

	// simulation function -- simulate from t to t+dt
	void sim_step(double dt);

};

// friction coefficient functions

double calculate_mu(double r);

double calculate_mu_bw(double r);
	

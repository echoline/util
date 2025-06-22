#define nil NULL

typedef struct Ann Ann;
typedef struct Layer Layer;
typedef struct Neuron Neuron;
typedef struct Weights Weights;

struct Ann {
	int n;
	double rate;
	Layer **layers;
	Weights **weights;
	Weights **deltas;
	void *user;
	void *internal;
};

struct Layer {
	int n;
	Neuron **neurons;
};

struct Neuron {
	double (*activation)(Neuron*);
	double (*gradient)(Neuron*);
	double steepness;
	double value;
	double sum;
	void *user;
	void *internal;
};

struct Weights {
	int inputs;
	int outputs;
	double **values;
};

double activation_sigmoid(Neuron*);
double gradient_sigmoid(Neuron*);
double activation_tanh(Neuron*);
double gradient_tanh(Neuron*);
double activation_leaky_relu(Neuron*);
double gradient_leaky_relu(Neuron*);
double activation_piece(Neuron*);
double gradient_piece(Neuron*);

Ann *anncreate(int, ...);
Ann *anncreatev(int, int*);
Layer *layercreate(int, double(*)(Neuron*), double(*)(Neuron*));
Neuron *neuroninit(Neuron*, double (*)(Neuron*), double (*)(Neuron*), double);
Neuron *neuroncreate(double (*)(Neuron*), double (*)(Neuron*), double);
Weights *weightsinitrand(Weights*);
Weights *weightsinitrandscale(Weights*, double);
Weights *weightsinitdouble(Weights*, double);
Weights *weightsinitdoubles(Weights*, double*);
Weights *weightscreate(int, int, int);
double *annrun(Ann*, double*);
double anntrain(Ann*, double*, double*);

typedef struct Adam Adam;

struct Adam {
	double rate;
	double beta1;
	double beta2;
	Weights **first;
	Weights **second;
	double epsilon;
	int timestep;
};

double anntrain_adam(Ann*, double*, double*);
double anntrain_adamax(Ann*, double*, double*);

int annsave(char *filename, Ann *ann);
Ann* annload(char *filename);
char *readline(int fd);
char *sreadline(int fd);

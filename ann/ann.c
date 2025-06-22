#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "ann.h"

#define ACTIVATION activation_leaky_relu
#define GRADIENT gradient_leaky_relu

double
activation_sigmoid(Neuron *in)
{
	return 1.0/(1.0+exp(-in->sum));
}

double
gradient_sigmoid(Neuron *in)
{
	double y = in->value;
	return y * (1.0 - y);
}

double
activation_tanh(Neuron *in)
{
	return tanh(in->sum);
}

double
gradient_tanh(Neuron *in)
{
	return 1.0 - in->value*in->value;
}

double
activation_leaky_relu(Neuron *in)
{
	if (in->sum > 0)
		return in->sum;
	return in->sum * 0.01;
}

double
gradient_leaky_relu(Neuron *in)
{
	if (in->sum > 0)
		return 1.0;
	return 0.01;
}

double
activation_piece(Neuron *in)
{
	if (in->sum < -0.5)
		return 0.0;
	else if (in->sum > 0.5)
		return 1.0;
	return (in->sum + 0.5);
}

double
gradient_piece(Neuron *in)
{
	if (in->sum > -0.5 && in->sum < 0.5)
		return 1.0;
	return 0.01;
}

Weights*
weightsinitdoubles(Weights *in, double *init)
{
	int i, o;

	for (i = 0; i <= in->inputs; i++)
		for (o = 0; o < in->outputs; o++)
			in->values[i][o] = init[o];

	return in;
}

Weights*
weightsinitdouble(Weights *in, double init)
{
	int i, o;

	for (i = 0; i <= in->inputs; i++)
		for (o = 0; o < in->outputs; o++)
			in->values[i][o] = init;

	return in;
}

Weights*
weightsinitrandscale(Weights *in, double scale)
{
	int i, o;

	srand(time(0));
	for (i = 0; i <= in->inputs; i++)
		for (o = 0; o < in->outputs; o++)
			in->values[i][o] = (((double)rand()/RAND_MAX) - 0.5) * scale;

	return in;
}

Weights*
weightsinitrand(Weights *in)
{
	weightsinitrandscale(in, 2.0);
	return in;
}

Neuron*
neuroninit(Neuron *in, double (*activation)(Neuron*), double (*gradient)(Neuron*), double steepness)
{
	in->activation = activation;
	in->gradient = gradient;
	in->steepness = steepness;
	in->value = 1.0;
	in->sum = 0;
	return in;
}

Neuron*
neuroncreate(double (*activation)(Neuron*), double (*gradient)(Neuron*), double steepness)
{
	Neuron *ret = calloc(1, sizeof(Neuron));
	neuroninit(ret, activation, gradient, steepness);
	return ret;
}

Layer*
layercreate(int num_neurons, double(*activation)(Neuron*), double(*gradient)(Neuron*))
{
	Layer *ret = calloc(1, sizeof(Layer));
	int i;

	ret->n = num_neurons;
	ret->neurons = calloc(num_neurons+1, sizeof(Neuron*));
	for (i = 0; i <= ret->n; i++) {
		ret->neurons[i] = neuroncreate(activation, gradient, 1.0);
	}
	return ret;
}

Weights*
weightscreate(int inputs, int outputs, int initialize)
{
	int i;
	Weights *ret = calloc(1, sizeof(Weights));
	ret->inputs = inputs;
	ret->outputs = outputs;
	ret->values = calloc(inputs+1, sizeof(double*));
	for (i = 0; i <= inputs; i++)
		ret->values[i] = calloc(outputs, sizeof(double));
	if (initialize)
		weightsinitrand(ret);
	return ret;
}

Ann*
anncreate(int num_layers, ...)
{
	Ann *ret = calloc(1, sizeof(Ann));
	va_list args;
	int arg;
	int i;

	va_start(args, num_layers);
	ret->n = num_layers;
	ret->rate = 0.7;
	ret->layers = calloc(num_layers, sizeof(Layer*));
	ret->weights = calloc(num_layers-1, sizeof(Weights*));
	ret->deltas = calloc(num_layers-1, sizeof(Weights*));

	for (i = 0; i < num_layers; i++) {
		arg = va_arg(args, int);
		if (arg < 0 || arg > 1000000)
			arg = 0;
		ret->layers[i] = layercreate(arg, ACTIVATION, GRADIENT);
		if (i > 0) {
			ret->weights[i-1] = weightscreate(ret->layers[i-1]->n, ret->layers[i]->n, 1);
			ret->deltas[i-1] = weightscreate(ret->layers[i-1]->n, ret->layers[i]->n, 0);
		}
	}

	va_end(args);

	return ret;
}

Ann*
anncreatev(int num_layers, int *layers)
{
	Ann *ret = calloc(1, sizeof(Ann));
	int arg;
	int i;

	ret->n = num_layers;
	ret->rate = 0.7;
	ret->layers = calloc(num_layers, sizeof(Layer*));
	ret->weights = calloc(num_layers-1, sizeof(Weights*));
	ret->deltas = calloc(num_layers-1, sizeof(Weights*));

	for (i = 0; i < num_layers; i++) {
		arg = layers[i];
		if (arg < 0 || arg > 1000000)
			arg = 0;
		if (i < (num_layers-1))
			ret->layers[i] = layercreate(arg, ACTIVATION, GRADIENT);
		else
			ret->layers[i] = layercreate(arg, activation_sigmoid, gradient_sigmoid);
		if (i > 0) {
			ret->weights[i-1] = weightscreate(ret->layers[i-1]->n, ret->layers[i]->n, 1);
			ret->deltas[i-1] = weightscreate(ret->layers[i-1]->n, ret->layers[i]->n, 0);
		}
	}

	return ret;
}

double*
annrun(Ann *ann, double *input)
{
	int l, i, o;
	int outputs = ann->layers[ann->n - 1]->n;
	double *ret = calloc(outputs, sizeof(double));
	Neuron *O;
	double sum;

	for (i = 0; i < ann->layers[0]->n; i++)
		ann->layers[0]->neurons[i]->value = input[i];

	for (l = 1; l < ann->n; l++) {
		for (o = 0; o < ann->layers[l]->n; o++) {
			O = ann->layers[l]->neurons[o];
			O->sum = ann->weights[l-1]->values[ann->weights[l-1]->inputs][o]; // bias
			sum = O->sum;
			#pragma omp parallel for reduction (+:sum)
			for (i = 0; i < ann->layers[l-1]->n; i++)
				sum += ann->layers[l-1]->neurons[i]->value * ann->weights[l-1]->values[i][o];
			if (sum < -300.0)
				sum = -300.0;
			else if (sum > 300.0)
				sum = 300.0;
			O->sum = sum;
			O->value = O->activation(O);
		}
	}

	for (o = 0; o < outputs; o++)
		ret[o] = ann->layers[ann->n - 1]->neurons[o]->value;

	return ret;
}

double
anntrain(Ann *ann, double *inputs, double *outputs)
{
	double *error = annrun(ann, inputs);
	double ret = 0.0;
	int noutputs = ann->layers[ann->n-1]->n;
	double acc, sum;
	int o, i, w, n;
	Neuron *O, *I;
	Weights *W, *D, *D2;

	for (o = 0; o < noutputs; o++) {
		// error = outputs[o] - result
		error[o] -= outputs[o];
		error[o] = -error[o];
		ret += pow(error[o], 2.0) * 0.5;
		if (error[o] < -.9999999)
			error[o] = -17.0;
		else if (error[o] > .9999999)
			error[o] = 17.0;
		else
			error[o] = log((1.0 + error[o]) / (1.0 - error[o]));
	}
	D = ann->deltas[ann->n-2];
	weightsinitdoubles(D, error);
	for (i = 0; i < (ann->n-2); i++) {
		D = ann->deltas[i];
		weightsinitdouble(D, 1.0);
	}

	// backpropagate MSE
	D2 = ann->deltas[ann->n-2];
	for (w = ann->n-2; w >= 0; w--) {
		D = ann->deltas[w];

		for (o = 0; o < ann->layers[w+1]->n; o++) {
			O = ann->layers[w+1]->neurons[o];
			acc = O->gradient(O) * O->steepness;
			sum = 1.0;
			if (D2 != D) {
				W = ann->weights[w + 1];
				sum = 0.0;
				#pragma omp parallel for reduction (+:sum)
				for (n = 0; n < D2->outputs; n++)
					sum += D2->values[o][n] * W->values[o][n];
			}
			for (i = 0; i <= ann->layers[w]->n; i++) {
			 	D->values[i][o] *= acc * sum;
			}
		}

		D2 = D;
	}

	// update weights
	for (w = 0; w < ann->n-1; w++) {
		W = ann->weights[w];
		D = ann->deltas[w];

		for (i = 0; i <= W->inputs; i++) {
			I = ann->layers[w]->neurons[i];
			for (o = 0; o < W->outputs; o++) {
				W->values[i][o] += D->values[i][o] * ann->rate * I->value;
			}
		}
	}

	free(error);
	return ret;
}

Ann*
adaminit(Ann *ann)
{
	int i;
	Adam *I = calloc(1, sizeof(Adam));

	I->rate = 0.001;
	I->beta1 = 0.9;
	I->beta2 = 0.999;
	I->epsilon = 10e-8;
	I->timestep = 0;
	I->first = calloc(ann->n-1, sizeof(Weights*));
	I->second = calloc(ann->n-1, sizeof(Weights*));

	for (i = 0; i < (ann->n-1); i++) {
		I->first[i] = weightscreate(ann->layers[i]->n, ann->layers[i+1]->n, 0);
		I->second[i] = weightscreate(ann->layers[i]->n, ann->layers[i+1]->n, 0);
	}

	ann->internal = I;

	return ann;
}

double
anntrain_adam(Ann *ann, double *inputs, double *outputs)
{
	double *error = annrun(ann, inputs);
	double ret = 0.0;
	int noutputs = ann->layers[ann->n-1]->n;
	double acc, sum, m, v;
	int o, i, w, n;
	Neuron *O, *I;
	Weights *W, *D, *D2, *M, *V;
	Adam *annI;

	if (ann->internal == 0)
		adaminit(ann);
	annI = ann->internal;
	annI->timestep++;

	for (o = 0; o < noutputs; o++) {
		// error = outputs[o] - result
		error[o] -= outputs[o];
		error[o] = -error[o];
		ret += pow(error[o], 2.0) * 0.5;
	}
	D = ann->deltas[ann->n-2];
	weightsinitdoubles(D, error);
	for (i = 0; i < (ann->n-2); i++) {
		D = ann->deltas[i];
		weightsinitdouble(D, 1.0);
	}

	// backpropagate MSE
	D2 = ann->deltas[ann->n-2];
	for (w = ann->n-2; w >= 0; w--) {
		D = ann->deltas[w];
		M = annI->first[w];
		V = annI->second[w];

		for (o = 0; o < ann->layers[w+1]->n; o++) {
			O = ann->layers[w+1]->neurons[o];
			acc = O->gradient(O) * O->steepness;
			sum = 1.0;
			if (D2 != D) {
				W = ann->weights[w+1];
				sum = 0.0;
				#pragma omp parallel for reduction (+:sum)
				for (n = 0; n < D2->outputs; n++)
					sum += D2->values[o][n] * W->values[o][n];
			}
			for (i = 0; i <= ann->layers[w]->n; i++) {
				I = ann->layers[w]->neurons[i];
			 	D->values[i][o] *= acc * sum;
				M->values[i][o] *= annI->beta1;
				M->values[i][o] += (1.0 - annI->beta1) * D->values[i][o] * I->value;
				V->values[i][o] *= annI->beta2;
				V->values[i][o] += (1.0 - annI->beta2) * D->values[i][o] * D->values[i][o] * I->value * I->value;
			}
		}

		D2 = D;
	}

	// update weights
	for (w = 0; w < ann->n-1; w++) {
		W = ann->weights[w];
		M = annI->first[w];
		V = annI->second[w];

		for (i = 0; i <= W->inputs; i++) {
			for (o = 0; o < W->outputs; o++) {
				m = M->values[i][o] / (annI->timestep < 100? (1.0 - pow(annI->beta1, annI->timestep)): 1.0);
				v = V->values[i][o] / (annI->timestep < 10000? (1.0 - pow(annI->beta2, annI->timestep)): 1.0);
				W->values[i][o] += (m / (sqrt(v) + annI->epsilon)) * annI->rate;
			}
		}
	}

	free(error);
	return ret;
}

double
anntrain_adamax(Ann *ann, double *inputs, double *outputs)
{
	double *error = annrun(ann, inputs);
	double ret = 0.0;
	int noutputs = ann->layers[ann->n-1]->n;
	double acc, sum, m, v;
	int o, i, w, n;
	Neuron *O, *I;
	Weights *W, *D, *D2, *M, *V;
	Adam *annI;

	if (ann->internal == 0)
		adaminit(ann);
	annI = ann->internal;
	annI->rate = 0.002;
	annI->timestep++;

	for (o = 0; o < noutputs; o++) {
		// error = outputs[o] - result
		error[o] -= outputs[o];
		error[o] = -error[o];
		ret += pow(error[o], 2.0) * 0.5;
	}
	D = ann->deltas[ann->n-2];
	weightsinitdoubles(D, error);
	for (i = 0; i < (ann->n-2); i++) {
		D = ann->deltas[i];
		weightsinitdouble(D, 1.0);
	}

	// backpropagate MSE
	D2 = ann->deltas[ann->n-2];
	for (w = ann->n-2; w >= 0; w--) {
		D = ann->deltas[w];
		M = annI->first[w];
		V = annI->second[w];

		for (o = 0; o < ann->layers[w+1]->n; o++) {
			O = ann->layers[w+1]->neurons[o];
			acc = O->gradient(O) * O->steepness;
			sum = 1.0;
			if (D2 != D) {
				W = ann->weights[w+1];
				sum = 0.0;
				#pragma omp parallel for reduction (+:sum)
				for (n = 0; n < D2->outputs; n++)
					sum += D2->values[o][n] * W->values[o][n];
			}
			for (i = 0; i <= ann->layers[w]->n; i++) {
				I = ann->layers[w]->neurons[i];
			 	D->values[i][o] *= acc * sum;
				M->values[i][o] *= annI->beta1;
				M->values[i][o] += (1.0 - annI->beta1) * D->values[i][o] * I->value;
				V->values[i][o] = fmax(V->values[i][o] * annI->beta2, fabs(D->values[i][o] * I->value));
			}
		}

		D2 = D;
	}

	// update weights
	for (w = 0; w < ann->n-1; w++) {
		W = ann->weights[w];
		M = annI->first[w];
		V = annI->second[w];

		for (i = 0; i <= W->inputs; i++) {
			for (o = 0; o < W->outputs; o++) {
				m = M->values[i][o];
				v = V->values[i][o];
				W->values[i][o] += (annI->rate/(1.0 - (annI->timestep < 100? pow(annI->beta1, annI->timestep): 0.0))) * (m/v);
			}
		}
	}

	free(error);
	return ret;
}

#include "HMM.h"


/*
 * TODO: Solve dependency error and move Callback/Momentum classes to separate files
 */



/*
 * Constructors
 */
HMM::HMM(int n_states, int  n_obs) { 
	N = n_states;
	M = n_obs;

	//create A,B,PI with correct dimensions
	A = std::vector<std::vector<double>>(N, std::vector<double>(N));
	B = std::vector<std::vector<double>>(N, std::vector<double>(M));
	PI = std::vector<std::vector<double>>(1, std::vector<double>(N));

	//set initial values
	initializeMatrix(A, Initialization::random, 0);
	initializeMatrix(B, Initialization::random, 0);
	initializeMatrix(PI, Initialization::random, 0);
}

HMM::HMM(const HMM& hmm) {
	N = hmm.N;
	M = hmm.M;

	setA(hmm.A);
	setB(hmm.B);
	setPI(hmm.PI);
}



/*
 *  Initialize the HMM with the chosen distribution and seed.
 *  This should probably be rewritten to use other C++ distributions
 */
//replace dist with stddev(and maybe mean?)
void HMM::initializeMatrix(std::vector<std::vector<double>>& matrix, Initialization dist, int seed) {
	//Discrete Uniform Distribution
	if (dist == Initialization::even) {
		for (int i = 0; i < matrix.size(); i++) {
			for (int j = 0; j < matrix[i].size(); j++) 
				matrix[i][j] = 1.0 / matrix[i].size();
		}
	}

	//Stamp Distribution
	//Use normal distribution with temperature?
	if (dist == Initialization::stamp) {
		srand(seed);
		for (int i = 0; i < matrix.size(); i++) {
			for (int j = 0; j < matrix[i].size(); j++) {
				matrix[i][j] = 1.0 / matrix[i].size();
				if (rand() & 0x1 == 0)
					matrix[i][j] = matrix[i][j] + (double)(rand() & 0x7) / 8.0 * matrix[i][j] / 10.0;
				else
					matrix[i][j] = matrix[i][j] - (double)(rand() & 0x7) / 8.0 * matrix[i][j] / 10.0;
				if (matrix[i][j] < 0)
					matrix[i][j] = 1.0 / matrix[i].size();
			}
		}

		for (int i = 0; i < matrix.size(); i++) {
			double sum = 0.0;
			for (int j = 0; j < matrix[i].size(); j++) 
				sum += matrix[i][j];
			for (int j = 0; j < matrix[i].size(); j++)
				matrix[i][j] /= sum;
		}
	}

	// Uniform Distribution
	if (dist == Initialization::random) {
		std::default_random_engine generator(seed);
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		for (int i = 0; i < matrix.size(); i++) {
			for (int j = 0; j < matrix[i].size(); j++) {
				matrix[i][j] = distribution(generator);
			}
		}

		for (int i = 0; i < matrix.size(); i++) {
			double sum = 0.0;
			for (int j = 0; j < matrix[i].size(); j++)
				sum += matrix[i][j];
			for (int j = 0; j < matrix[i].size(); j++)
				matrix[i][j] /= sum;
		}
	}
	
}


/*
 *  Initialize A matrix
 */
void HMM::initializeA(Initialization dist, int seed) {
	initializeMatrix(A, dist, seed);
}

/*
 *  Initialize B matrix
 */
void HMM::initializeB(Initialization dist, int seed) {
	initializeMatrix(B, dist, seed);
}

/*
 *  Initialize Pi matrix
 */
void HMM::initializePi(Initialization dist, int seed) {
	initializeMatrix(PI, dist, seed);
}



/*
 *  Setters/getters
 */
int HMM::getN() {
	return N;
}

int HMM::getM() {
	return M;
}

std::vector<std::vector<double>>& HMM::getA() {
	return A;
}

std::vector<std::vector<double>>& HMM::getB() {
	return B;
}

std::vector<std::vector<double>>& HMM::getPi() {
	return PI;
}


bool HMM::setA(const std::vector<std::vector<double>>& _A) {
	if (_A.size() != N)
		return false;
	for (auto a : _A)
		if (a.size() != N)
			return false;
	A = _A;
	return true;
}

bool HMM::setB(const std::vector<std::vector<double>>& _B) {
	if (_B.size() != N)
		return false;
	for (auto b : _B)
		if (b.size() != M)
			return false;
	B = _B;
	return true;
}

bool HMM::setPI(const std::vector<std::vector<double>>& _PI) {
	if (_PI.size() != 1)
		return false;
	for (auto pi : _PI)
		if (pi.size() != N)
			return false;
	PI = _PI;
	return true;
}







/*
 *  Train the HMM
 */
std::vector<double> HMM::train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks, bool trainA, bool trainB, bool trainPi) {
	if (observations.size() == 0) {
		std::cout << "Error: observation sequence was empty" << std::endl;
		return std::vector<double>();
	}

	std::cout << "momentum = " << momentum.getMomentum() << std::endl;

	int T = 0;
	for (const auto& O : observations) 
		T += O.size();
	
	double oldLogProb = -DBL_MAX;
	double logProb = 0.0;
	std::vector<double> history;

	momentum.onTrainStart(*this, max_iters);

	for (Callback* callback : callbacks)
		callback->onTrainStart(*this, max_iters);

	int iters = 0;
	while (iters < max_iters) {
		for (Callback* callback : callbacks)
			callback->onIterStart(*this, iters);

		//reset score
		oldLogProb = logProb;		
		logProb = 0.0;

		momentum.onIterStart(*this, iters);

		std::vector<double> numerPi(N, 0.0);
		double denomPi = 0;
		std::vector<std::vector<double>> numerA(N, std::vector<double>(N,0.0));
		std::vector<double> denomA(N,0.0);
		std::vector<std::vector<double>> numerB(N, std::vector<double>(M, 0.0));;
		std::vector<double> denomB(N,0.0);
		for (auto& O : observations) {
			int O_T = O.size();
			alphas = std::vector<std::vector<double>>(O_T, std::vector<double>(N));
			betas = std::vector<std::vector<double>>(O_T, std::vector<double>(N));
			gammas = std::vector<std::vector<double>>(O_T, std::vector<double>(N));
			digammas = std::vector<std::vector<std::vector<double>>>(O_T);
			for (int t = 0; t < O_T; t++)
				digammas[t] = std::vector<std::vector<double>>(N, std::vector<double>(N));
			constants = std::vector<double>(O_T, 0.0);

			alphaPass(O, O_T);			//alpha(forward) pass
			betaPass(O, O_T);			//beta(backwards) pass
			computeGammas(O, O_T);		//calculate gammas & digammas

			momentum.onUpdateStart(*this, iters);

			//Computing score for BW before smoothing/momentum
			for (int t = 0; t < O_T; t++)
				logProb += log(constants[t]);

			//TODO: Move reestimation into own functions
			//Estimate Pi
			for (int i = 0; i < N; i++)
				numerPi[i] += gammas[0][i];
			denomPi += 1;

			//Estimate A
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					for (int t = 0; t < O_T - 1; t++)
						numerA[i][j] += digammas[t][i][j];
				}
				for (int t = 0; t < O_T - 1; t++)
					denomA[i] += gammas[t][i];
			}

			//Estimate B
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < M; j++) {
					for (int t = 0; t < O_T - 1; t++) {
						if (O[t] == j)
							numerB[i][j] += gammas[t][i];
					}
				}
				for (int t = 0; t < O_T - 1; t++)
					denomB[i] += gammas[t][i];
			}
		}

		if (trainPi)
			updatePi(numerPi, denomPi, smoothing);
		if (trainA)
			updateA(numerA, denomA, smoothing);
		if (trainB)
			updateB(numerB, denomB, smoothing);

		momentum.onUpdateEnd(*this, iters);

		//Negate cumulative logProb from all observation sequences
		logProb = -logProb;

		//std::cout << iters << " - " << std::setprecision(6) << logProb << std::endl;
		printf("iter %d = %f\n", iters, logProb);
		// std::cout << s_entropy() << std::endl;
		history.push_back(logProb);

		momentum.onIterEnd(*this, iters, logProb);

		for (Callback* callback : callbacks)
			callback->onIterEnd(*this, iters, logProb);

		iters++;

	}

	momentum.onTrainEnd(*this, iters, logProb);

	for (Callback* callback : callbacks)
		callback->onTrainEnd(*this, iters, logProb);

	return history;


}


void HMM::updatePi(std::vector<double> numers, double denom, double smoothing) {
	//Smooth Pi
	if (smoothing != 0.0) {
		for (int i = 0; i < N; i++)
			numers[i] += smoothing;
		denom += N * smoothing;
	}
	

	for (int i = 0; i < N; i++) {
		double newProb = numers[i] / denom;
		PI[0][i] = newProb;
	}
}

void HMM::updateA(std::vector<std::vector<double>> numers, std::vector<double> denoms, double smoothing) {
	//Smooth A
	if (smoothing != 0.0) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++)
				numers[i][j] += smoothing;
			denoms[i] = (denoms[i] + (N * smoothing));
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			double newProb = numers[i][j] / denoms[i];  //get new BW probability at A[i][j] 
			A[i][j] = newProb;
		}
	}
}


void HMM::updateB(std::vector<std::vector<double>> numers, std::vector<double> denoms, double smoothing) {
	//Smooth B
	if (smoothing != 0.0) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++)
				numers[i][j] += smoothing;
			denoms[i] = (denoms[i] + (M * smoothing));
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			double newProb = numers[i][j] / denoms[i];
			B[i][j] = newProb;
		}
	}
}




//forward pass
void HMM::alphaPass(const std::vector<int>& O, int T) {
	//reinitialize all constants to 0.0
	for (int t = 0; t < T; t++)
		constants[t] = 0.0;

	//compute a0(i) for each i in N
	for (int i = 0; i < N; i++) {
		alphas[0][i] = PI[0][i] * B[i][O[0]];
		constants[0] += alphas[0][i];
	}
	//scale a0(i) for each i in N
	constants[0] = 1.0 / constants[0];
	for (int i = 0; i < N; i++)
		alphas[0][i] *= constants[0];

	//compute at(i) for rest of t in T and each i in N
	for (int t = 1; t < T; t++) {
		for (int i = 0; i < N; i++) {
			alphas[t][i] = 0;
			for (int j = 0; j < N; j++)
				alphas[t][i] += alphas[t - 1][j] * A[j][i];
			alphas[t][i] *= B[i][O[t]];
			constants[t] += alphas[t][i];
		}

		//scale at t
		constants[t] = 1.0 / constants[t];
		for (int i = 0; i < N; i++)
			alphas[t][i] *= constants[t];
	}
}


//backwards pass
void HMM::betaPass(const std::vector<int>& O, int T) {
	//set last beta
	for (int i = 0; i < N; i++)
		betas[T - 1][i] = constants[T - 1];

	//for rest, starting at 2nd last and moving backwards
	//prob of each transition * prob of state mapping to each obs * prob of rest of sequence
	for (int t = T - 2; t >= 0; t--) {
		for (int i = 0; i < N; i++) {
			betas[t][i] = 0;
			for (int j = 0; j < N; j++)
				betas[t][i] += A[i][j] * B[j][O[t + 1]] * betas[t + 1][j];
			betas[t][i] *= constants[t];
		}
	}

}


//Check this in detail
//Is the scaling by denom needed?
void HMM::computeGammas(const std::vector<int>& O, int T) {
	double denom = 0.0;
	for (int t = 0; t < T - 1; t++) {
		denom = 0.0;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				digammas[t][i][j] = alphas[t][i] * A[i][j] * B[j][O[t + 1]] * betas[t + 1][j];
				denom += digammas[t][i][j];
			}
		}

		for (int i = 0; i < N; i++) {
			gammas[t][i] = 0;
			for (int j = 0; j < N; j++) {
				digammas[t][i][j] /= denom;
				gammas[t][i] += digammas[t][i][j];
			}
		}
	}

	//special case for t = T-1
	denom = 0.0;
	for (int i = 0; i < N; i++)
		denom += alphas[T - 1][i];

	for (int i = 0; i < N; i++)
		gammas[T - 1][i] = alphas[T - 1][i] / denom;

}





double HMM::score(const std::vector<int>& O) {
	int T = O.size();

	std::vector<std::vector<double>> alphas = std::vector<std::vector<double>>(T, std::vector<double>(N));
	std::vector<double> constants = std::vector<double>(T, 0.0);

	//reinitialize all constants to 0.0
	for (int t = 0; t < T; t++)
		constants[t] = 0.0;

	//compute a0(i) for each i in N
	for (int i = 0; i < N; i++) {
		alphas[0][i] = PI[0][i] * B[i][O[0]];
		constants[0] += alphas[0][i];
	}
	//scale a0(i) for each i in N
	constants[0] = 1.0 / constants[0];
	for (int i = 0; i < N; i++)
		alphas[0][i] *= constants[0];

	//compute at(i) for rest of t in T and each i in N
	for (int t = 1; t < T; t++) {
		for (int i = 0; i < N; i++) {
			alphas[t][i] = 0;
			for (int j = 0; j < N; j++)
				alphas[t][i] += alphas[t - 1][j] * A[j][i];
			alphas[t][i] *= B[i][O[t]];
			constants[t] += alphas[t][i];
		}

		//scale at t
		constants[t] = 1.0 / constants[t];
		for (int i = 0; i < N; i++)
			alphas[t][i] *= constants[t];
	}


	double logProb = 0.0;
	for (int t = 0; t < T; t++)
		logProb += log(constants[t]);
	logProb = -logProb;


	return logProb;
}


double HMM::normalizedScore(const std::vector<int>& O) {
	int T = O.size();

	return score(O) / T;
}




/*
 *  Train overloads
 */
std::vector<double> HMM::train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks) {
	return train(observations, max_iters, momentum, smoothing, callbacks, true, true, true);
}


std::vector<double> HMM::train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing) {
	std::vector<Callback*> c;
	return train(observations, max_iters, momentum, smoothing, c);
}


std::vector<double> HMM::train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum) {
	std::vector<Callback*> c;
	return train(observations, max_iters, momentum, 0, c);
}


/*
 *  Train overloads, single sequence
 */
std::vector<double> HMM::train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks, bool trainA, bool trainB, bool trainPi) {
	std::vector<std::vector<int>> O;
	O.push_back(observations);
	return train(O, max_iters, momentum, smoothing, callbacks, trainA, trainB, trainPi);
}


std::vector<double> HMM::train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks) {
	std::vector<std::vector<int>> O;
	O.push_back(observations);
	return train(O, max_iters, momentum, smoothing, callbacks, true, true, true);
}


std::vector<double> HMM::train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing) {
	std::vector<Callback*> c;
	return train(observations, max_iters, momentum, smoothing, c, true, true, true);
}


std::vector<double> HMM::train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum) {
	return train(observations, max_iters, momentum, 0);
}






//(shannon) entropy 
//not entirely sure how I should do this
//trying to just apply the formula to each row and and sum or average for total entropy score
//sum across all matrices too?
//or should you use O also?
double HMM::s_entropy() {
	//double entropy = -np.sum(pA*np.log2(pA))
	double entropy = 0;
	for (int i = 0; i < N; i++) {
		double row_entropy = 0;
		for (int j = 0; j < N; j++) {
			double p = A[i][j];
			if (p == 0)
				p = 0.000000001;
			row_entropy += p * log2(p);
		}
		row_entropy *= -1;
		entropy += row_entropy;
	}

	for (int i = 0; i < N; i++) {
		double row_entropy = 0;
		for (int j = 0; j < M; j++) {
			double p = B[i][j];
			if (p == 0)
				p = 0.000000001;
			row_entropy += p * log2(p);
		}
		row_entropy *= -1;
		entropy += row_entropy;
	}

	return entropy;
}





//Assumes no negatives, and will have issues if entire vector is of 0.0
void HMM::normalize(std::vector<double>& vec) {
	double sum = 0.0;
	for (int i = 0; i < vec.size(); i++) {
		//if (vec[i] < 0)
		//	vec[i] = 0.000001;
		sum += vec[i];
	}

	//zero vector
	if (sum == 0.0) {
		std::cout << "ERROR: trying to normalize a zero vector" << std::endl;
		exit(1);
	}

	for (int i = 0; i < vec.size(); i++)
		vec[i] /= sum;
}






// Train via Gradient descent.  Somewhat working, but WIP
std::vector<double> HMM::trainGD(std::vector<int>& observations, unsigned int max_iters, double lr, double momentum, double temp, 
									bool trainA, bool trainB, bool trainPi, bool useNesterov) {

	if (observations.size() == 0) {
		std::cout << "Error: observation sequence was empty" << std::endl;
		return std::vector<double>();
	}

	this->momentum = momentum;
	this->nesterov = useNesterov;

	std::vector<int>& O = observations;
	int T = observations.size();

	//Setup training matrices
	//Should really make some sort of class for each t in T.  Timestep...?
	//save matrices as private members for looking at values after? or create inside train?
	alphas = std::vector<std::vector<double>>(T, std::vector<double>(N));
	betas = std::vector<std::vector<double>>(T, std::vector<double>(N));
	gammas = std::vector<std::vector<double>>(T, std::vector<double>(N));
	digammas = std::vector<std::vector<std::vector<double>>>(T);
	for (int t = 0; t < T; t++)
		digammas[t] = std::vector<std::vector<double>>(N, std::vector<double>(N));
	constants = std::vector<double>(T, 0.0);

	momentumA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));
	momentumB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));
	momentumPi = std::vector<std::vector<double>>(1, std::vector<double>(N, 0.0));

	std::vector<std::vector<double>> weightsA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));;
	std::vector<std::vector<double>> weightsB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));;


	double oldLogProb = -DBL_MAX;
	double logProb = 0.0;
	std::vector<double> history;

	double targetMomentum = momentum;

	int iters = 0;
	while (iters < max_iters) {
		if (iters < 1 || iters >= max_iters - 10)
			momentum = 0.0;//targetMomentum / 9;
		else
			momentum = this->momentum;

		alphaPass(O, T);			//alpha(forward) pass
		betaPass(O, T);				//beta(backwards) pass
		computeGammas(O, T);		//calculate gammas & digammas

		//computeGradients();
		//updateWeights();  //update W,V with step towards gradient
		//updateModel();	//update A,B using W,V and softmax
		/*if (iters == 100) //90, 200?
			lr /= 2;
		if (iters == 300)
			lr /= 2;*/

		updateGradientDescentWeightsA(weightsA, lr, temp, momentum);
		updateGradientDescentWeightsB(weightsB, lr, temp, momentum, O);

		gradientDescentUpdateA(weightsA, temp, momentum);
		gradientDescentUpdateB(weightsB, temp, momentum);
		for (int i = 0; i < N; i++) 
			PI[0][i] = gammas[0][i];
		//normalize(PI[0]);

		oldLogProb = logProb;		//score
		logProb = 0.0;
		for (int t = 0; t < T; t++)
			logProb += log(constants[t]);
		logProb = -logProb;

		//std::cout << iters << " - " << std::setprecision(6) << logProb << std::endl;
		printf("iter %d = %f\n", iters, logProb);
		history.push_back(logProb);

		iters++;
	}

	return history;
}

void HMM::updateGradientDescentWeightsA(std::vector<std::vector<double>>& weightsA, double lr, double temp, double momentum) {
	int T = gammas.size();

	double logProb = 0.0;
	for (int t = 0; t < T; t++)
		logProb += log(constants[t]);
	//logProb = -logProb; //not sure if this is supposed to be included or not
	double C = logProb;

	for (int i = 0; i < N; i++) {
		//Sum total for i. This will be true for i, any j
		double expected_total = 0.0;	//expected number of times transitioning out of state i
		for (int t = 0; t < T - 1; t++)
			expected_total += gammas[t][i];

		for (int j = 0; j < N; j++) {
			//Sum transitions for i->j. 
			double expected_transitions = 0.0;	//expected number of times transitioning from state i to state j
			for (int t = 0; t < T - 1; t++) 
				expected_transitions += digammas[t][i][j];
			
			//Update index i,j
			double dir = (expected_transitions - (expected_total * A[i][j])) / C;
			double step = dir * lr * temp;
			weightsA[i][j] += step;

			//Add Momentum
			//if (!nesterov)
				weightsA[i][j] += momentumA[i][j];

			//Update Momentum
			momentumA[i][j] += step;
			momentumA[i][j] *= momentum;
		}
	}

}

void HMM::updateGradientDescentWeightsB(std::vector<std::vector<double>>& weightsB, double lr, double temp, double momentum, std::vector<int>& O) {
	int T = O.size();

	double logProb = 0.0;
	for (int t = 0; t < T; t++)
		logProb += log(constants[t]);
	//logProb = -logProb; //not sure if this is supposed to be included or not
	double C = logProb;

	for (int i = 0; i < N; i++) {
		double expected_total = 0.0;	
		for (int t = 0; t <= T - 1; t++)
			expected_total += gammas[t][i];

		for (int j = 0; j < M; j++) {
			double expected_emissions = 0.0;	
			for (int t = 0; t <= T - 1; t++) {
				if (O[t] == j)
					expected_emissions += gammas[t][i];
			}
				
			//Update index i,j
			double dir = (expected_emissions - (expected_total * B[i][j])) / C;
			double step = dir * lr * temp;
			weightsB[i][j] += step;

			//Add Momentum
			//if (!nesterov)
				weightsB[i][j] += momentumB[i][j];

			//Update Momentum
			momentumB[i][j] += step;
			momentumB[i][j] *= momentum;
		}
	}

}

void HMM::gradientDescentUpdateA(std::vector<std::vector<double>>& weightsA, double temp, double momentum) {
	for (int i = 0; i < N; i++) {
		double sum = 0.0;
		for (int k = 0; k < N; k++) 
			sum += exp(temp * weightsA[i][k]);

		for (int j = 0; j < N; j++) {
			A[i][j] = exp(temp * weightsA[i][j]) / sum;
		}
	}
}

//Combine these into one gradientDescentMatrixUpdate function that takes matrix, weights, dims as params
void HMM::gradientDescentUpdateB(std::vector<std::vector<double>>& weightsB, double temp, double momentum) {
	for (int i = 0; i < N; i++) {
		double sum = 0.0;
		for (int k = 0; k < M; k++)
			sum += exp(temp * weightsB[i][k]);

		for (int j = 0; j < M; j++) {
			B[i][j] = exp(temp * weightsB[i][j]) / sum;
		}
	}

}



//void HMM::updateWeights(std::vector<std::vector<double>>& weightsA, std::vector<std::vector<double>>& weightsB, double lr, double temp, double momentum) {}



Callback::Callback() {
}

void Callback::onTrainStart(HMM& hmm, int max_iters) {
}

void Callback::onTrainEnd(HMM& hmm, int iter, double score) {
}

void Callback::onIterStart(HMM&, int iter) {
}

void Callback::onIterEnd(HMM&, int iter, double score) {
}

void Callback::onUpdateStart(HMM& hmm, int iter) {
}

void Callback::onUpdateEnd(HMM& hmm, int iter) {
}


std::vector<std::vector<double>>& Callback::getMomentumA(HMM& hmm) {
	return hmm.momentumA;
}

std::vector<std::vector<double>>& Callback::getMomentumB(HMM& hmm) {
	return hmm.momentumB;
}

std::vector<std::vector<double>>& Callback::getMomentumPi(HMM& hmm) {
	return hmm.momentumPi;
}







Momentum::Momentum(double m) {
	momentum = m;
	schedule = NULL;
}

double Momentum::getMomentum() {
	return momentum;
}

void Momentum::setMomentum(double m) {
	momentum = m;
}

void Momentum::clear() {
	for (auto& row : momentumA)
		for (double& d : row)
			d = 0;
	for (auto& row : momentumB)
		for (double& d : row)
			d = 0;
	for (auto& row : momentumPi)
		for (double& d : row)
			d = 0;
}

void Momentum::addSchedule(MomentumSchedule& sched) {
	schedule = &sched;
}




ClassicMomentum::ClassicMomentum(double m) : Momentum(m) {
}

void ClassicMomentum::onTrainStart(HMM& hmm, int max_iters) {
	int N = hmm.getN();
	int M = hmm.getM();
	momentumA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));
	momentumB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));
	momentumPi = std::vector<std::vector<double>>(1, std::vector<double>(N, 0.0));

	prevA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));
	prevB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));
	prevPi = std::vector<std::vector<double>>(1, std::vector<double>(N, 0.0));

	deltaA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));
	deltaB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));
	deltaPi = std::vector<std::vector<double>>(1, std::vector<double>(N, 0.0));

	if (schedule != NULL)
		schedule->onTrainStart(hmm, *this, max_iters);
}

void ClassicMomentum::onTrainEnd(HMM& hmm, int iter, double score) {
	if (schedule != NULL)
		schedule->onTrainEnd(hmm, *this, iter, score);
}

void ClassicMomentum::onIterStart(HMM& hmm, int iter) {
	if (schedule != NULL)
		schedule->onIterStart(hmm, *this, iter);
}

void ClassicMomentum::onIterEnd(HMM& hmm, int iter, double score) {
	if (schedule != NULL)
		schedule->onIterEnd(hmm, *this, iter, score);
}

void ClassicMomentum::onUpdateStart(HMM& hmm, int iter) {
	prevA = hmm.getA();
	prevB = hmm.getB();
	prevPi = hmm.getPi();

	if (schedule != NULL)
		schedule->onUpdateStart(hmm, *this, iter);
}

void ClassicMomentum::onUpdateEnd(HMM& hmm, int iter) {
	int N = hmm.getN();
	int M = hmm.getM();
	auto& A = hmm.getA();
	auto& B = hmm.getB();
	auto& PI = hmm.getPi();

	if (schedule != NULL)
		schedule->onUpdateEnd(hmm, *this, iter);

	//Compute change for each element
	for (int i = 0; i < N; i++)
		deltaPi[0][i] = PI[0][i] - prevPi[0][i];
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			deltaA[i][j] = A[i][j] - prevA[i][j];
		}
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			deltaB[i][j] = B[i][j] - prevB[i][j];
		}
	}

	//apply momentum
	applyMomentum(hmm);

	//update momentum based on changes
	updateMomentum(hmm);

}



void ClassicMomentum::applyMomentum(HMM& hmm) {
	int N = hmm.getN();
	int M = hmm.getM();
	auto& A = hmm.getA();
	auto& B = hmm.getB();
	auto& PI = hmm.getPi();

	for (int i = 0; i < N; i++) {
		PI[0][i] += momentumPi[0][i];
		if (PI[0][i] < 0)
			PI[0][i] = e; 
	}
	normalize(PI[0]); //required due to momentum

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			A[i][j] += momentumA[i][j];
			if (A[i][j] < 0)
				A[i][j] = e;
		}
		normalize(A[i]);
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			B[i][j] += momentumB[i][j];
			if (B[i][j] < 0)
				B[i][j] = e; 
		}
		normalize(B[i]);
	}
}





void ClassicMomentum::updateMomentum(HMM& hmm) {
	int N = hmm.getN();
	int M = hmm.getM();
	auto& A = hmm.getA();
	auto& B = hmm.getB();
	auto& PI = hmm.getPi();

	for (int i = 0; i < N; i++) {
		//calculate new accumulated "discrete gradient" for lack of a better term
		momentumPi[0][i] += deltaPi[0][i];
		//scale accumulated gradient by momentum
		momentumPi[0][i] *= momentum;
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			momentumA[i][j] += deltaA[i][j];
			momentumA[i][j] *= momentum;
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			momentumB[i][j] += deltaB[i][j];
			momentumB[i][j] *= momentum;
		}
	}
}




NesterovMomentum::NesterovMomentum(double m) : Momentum(m) {
}

void NesterovMomentum::onTrainStart(HMM& hmm, int max_iters) {
	int N = hmm.getN();
	int M = hmm.getM();
	momentumA = std::vector<std::vector<double>>(N, std::vector<double>(N, 0.0));
	momentumB = std::vector<std::vector<double>>(N, std::vector<double>(M, 0.0));
	momentumPi = std::vector<std::vector<double>>(1, std::vector<double>(N, 0.0));

	if (schedule != NULL)
		schedule->onTrainStart(hmm, *this, max_iters);
}

void NesterovMomentum::onTrainEnd(HMM& hmm, int iter, double score) {
	if (schedule != NULL)
		schedule->onTrainEnd(hmm, *this, iter, score);
}

void NesterovMomentum::onIterStart(HMM& hmm, int iter) {
	prevA = hmm.getA();
	prevB = hmm.getB();
	prevPi = hmm.getPi();

	if (schedule != NULL)
		schedule->onIterStart(hmm, *this, iter);

	applyNesterov(hmm);
}

void NesterovMomentum::onIterEnd(HMM& hmm, int iter, double score) {
	if (schedule != NULL)
		schedule->onIterEnd(hmm, *this, iter, score);
}

void NesterovMomentum::onUpdateStart(HMM& hmm, int iter) {
	if (schedule != NULL)
		schedule->onUpdateStart(hmm, *this, iter);
}

void NesterovMomentum::onUpdateEnd(HMM& hmm, int iter) {
	if (schedule != NULL)
		schedule->onUpdateEnd(hmm, *this, iter);

	updateNesterov(hmm);

}

void NesterovMomentum::applyNesterov(HMM& hmm) {
	int N = hmm.getN();
	int M = hmm.getM();
	auto& A = hmm.getA();
	auto& B = hmm.getB();
	auto& PI = hmm.getPi();

	//A
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			A[i][j] += momentumA[i][j];
			if (A[i][j] < 0) {
				A[i][j] = e;
			}
		}
		normalize(A[i]);
	}
	//B
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			B[i][j] += momentumB[i][j];
			if (B[i][j] < 0) {
				B[i][j] = e;
			}
		}
		normalize(B[i]);
	}
	//Pi
	for (int i = 0; i < N; i++) {
		PI[0][i] += momentumPi[0][i];
		if (PI[0][i] < 0) {
			PI[0][i] = e;
		}
	}
	normalize(PI[0]);
}


void NesterovMomentum::updateNesterov(HMM& hmm) {
	int N = hmm.getN();
	int M = hmm.getM();
	auto& A = hmm.getA();
	auto& B = hmm.getB();
	auto& PI = hmm.getPi();

	for (int i = 0; i < N; i++) {
		momentumPi[0][i] += PI[0][i] - prevPi[0][i]; 
		momentumPi[0][i] *= momentum;
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			momentumA[i][j] += A[i][j] - prevA[i][j];
			momentumA[i][j] *= momentum;
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			momentumB[i][j] += B[i][j] - prevB[i][j];
			momentumB[i][j] *= momentum;
		}
	}
}


#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <math.h>
#include <iomanip>
#include <random>
#include <algorithm>
#include "util.h"
#include "schedule.h"


/*
 * TODO: Split Momentum classes into own file without dependency errors
 */


//enum class Momentum { none, momentum, nesterov };
enum class Initialization { Even, Stamp, Random };

class MomentumSchedule;
class Callback;
class Momentum;

class HMM
{
public:
	// Constructors
	HMM(int n_states, int n_obs);
	HMM(const HMM&);

	//Need to clean these up/actually think about options
	std::vector<double> train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks, bool trainA, bool trainB, bool trainPi);
	std::vector<double> train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks);
	std::vector<double> train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum, double smoothing);
	std::vector<double> train(std::vector<std::vector<int>>& observations, unsigned int max_iters, Momentum& momentum);

	std::vector<double> train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks, bool reestimateA, bool reestimateB, bool reestimatePI);
	std::vector<double> train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing, std::vector<Callback*>& callbacks);
	std::vector<double> train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum, double smoothing);
	std::vector<double> train(std::vector<int>& observations, unsigned int max_iters, Momentum& momentum);

	std::vector<double> trainGD(std::vector<int>& observations, unsigned int iters, double lr, double momentum, double temp, bool reestimateA, bool reestimateB, bool reestimatePI, bool useNesterov);
	//void updateWeights(std::vector<std::vector<double>>& weightsA, std::vector<std::vector<double>>& weightsB, double lr, double temp, double momentum);

	//void initializeMatrices(int dist, int seed);
	void initializeA(Initialization dist, int seed);
	void initializeB(Initialization dist, int seed);
	void initializePi(Initialization dist, int seed);

	double score(const std::vector<int>& O);
	double normalizedScore(const std::vector<int>& O);

	int getN();
	int getM();
	std::vector<std::vector<double>>& getA();
	std::vector<std::vector<double>>& getB();
	std::vector<std::vector<double>>& getPi();
	bool setA(const std::vector<std::vector<double>>&);
	bool setB(const std::vector<std::vector<double>>&);
	bool setPI(const std::vector<std::vector<double>>&);

	double s_entropy();




private:
	void initializeMatrix(std::vector<std::vector<double>>& matrix, Initialization dist, int seed);

	void alphaPass(const std::vector<int>& O, int T);
	void betaPass(const std::vector<int>& O, int T);
	void computeGammas(const std::vector<int>& O, int T);

	//void reestimateA(const std::vector<int>& O, int T, double momentum, double smoothing);
	//void reestimateB(const std::vector<int>& O, int T, double momentum, double smoothing);
	//void reestimatePi(const std::vector<int>& O, int T, double momentum, double smoothing);
	void updateA(std::vector<std::vector<double>> numers, std::vector<double> denoms, double smoothing);
	void updateB(std::vector<std::vector<double>> numers, std::vector<double> denoms, double smoothing);
	void updatePi(std::vector<double> numers, double denom, double smoothing);

	int N;   //number of hidden states
	int M;   //number of possible observations 

	//These are not contiguous in memory. 
	std::vector<std::vector<double>> A;  //transition matrix
	std::vector<std::vector<double>> B;  //observation probability matrix
	std::vector<std::vector<double>> PI; //initial state matrix

	//CONSIDER MAKING SOME SORT OF CLASS FOR EACH T
	//SO vector<???> of size T, where each ??? has an alpha/beta/gamma/digamma of size N
	std::vector<std::vector<double>> alphas;
	std::vector<std::vector<double>> betas;
	std::vector<std::vector<double>> gammas;
	std::vector<std::vector<std::vector<double>>> digammas;
	std::vector<double> constants;

	void normalize(std::vector<double>& vec);

	friend class Callback;
	friend class Momentum;


	// For GD.  Need to add momentum class integration for GD still.
	void updateGradientDescentWeightsA(std::vector<std::vector<double>>& weightsA, double lr, double temp, double momentum);
	void updateGradientDescentWeightsB(std::vector<std::vector<double>>& weightsB, double lr, double temp, double momentum, std::vector<int>& O);
	void gradientDescentUpdateA(std::vector<std::vector<double>>& weightsA, double temp, double momentum);
	void gradientDescentUpdateB(std::vector<std::vector<double>>& weightsB, double temp, double momentum);

	double momentum = 0.0;
	bool nesterov = false;
	std::vector<std::vector<double>> momentumA;
	std::vector<std::vector<double>> momentumB;
	std::vector<std::vector<double>> momentumPi;
	std::vector<std::vector<double>> prevA;
	std::vector<std::vector<double>> prevB;
	std::vector<std::vector<double>> prevPi;
};






class Callback {
public:
	Callback();
	virtual void onTrainStart(HMM& hmm, int max_iters); //before 1st iter
	virtual void onTrainEnd(HMM& hmm, int iter, double score);		//after last iter
	virtual void onIterStart(HMM& hmm, int iter);	 //at start of each iter
	virtual void onIterEnd(HMM& hmm, int iter, double score);	 //at end of each iter
	virtual void onUpdateStart(HMM& hmm, int iter); //after????? calc gammas, before reestimate
	virtual void onUpdateEnd(HMM& hmm, int iter); //after reestimate
	//virtual void preSmooth
	//virtual void postSmooth
	//virtual void preMomentum
	//virtual void postMomentum


protected:
	std::vector<std::vector<double>>& getMomentumA(HMM& hmm);
	std::vector<std::vector<double>>& getMomentumB(HMM& hmm);
	std::vector<std::vector<double>>& getMomentumPi(HMM& hmm);

private:
	//HMM& _hmm;
};







// Could make this pure virtual?
class Momentum : public Callback {
public:
	Momentum(double m);
	//Momentum(Callback& schedule)

	double getMomentum();
	void setMomentum(double m);
	virtual void clear();

	void addSchedule(MomentumSchedule& sched);

	//virtual std::vector<std::vector<double>>& getMomentumA();
	//virtual std::vector<std::vector<double>>& getMomentumB();
	//virtual std::vector<std::vector<double>>& getMomentumPi();

	MomentumSchedule* schedule;

protected:
	double momentum;

	std::vector<std::vector<double>> momentumA;
	std::vector<std::vector<double>> momentumB;
	std::vector<std::vector<double>> momentumPi;

	double e = 0.000000001;

	//HMM* _hmm;
};



class ClassicMomentum : public Momentum {
public:
	ClassicMomentum(double m);

	void onTrainStart(HMM& hmm, int max_iters);
	void onTrainEnd(HMM& hmm, int iter, double score);
	void onIterStart(HMM& hmm, int iter);
	void onIterEnd(HMM& hmm, int iter, double score);
	void onUpdateStart(HMM& hmm, int iter);
	void onUpdateEnd(HMM& hmm, int iter);

private:
	void applyMomentum(HMM& hmm);
	void updateMomentum(HMM& hmm);

	std::vector<std::vector<double>> deltaA;
	std::vector<std::vector<double>> deltaB;
	std::vector<std::vector<double>> deltaPi;

	std::vector<std::vector<double>> prevA;
	std::vector<std::vector<double>> prevB;
	std::vector<std::vector<double>> prevPi;
};



class NesterovMomentum : public Momentum {
public:
	NesterovMomentum(double m);

	void onTrainStart(HMM& hmm, int max_iters);
	void onTrainEnd(HMM& hmm, int iter, double score);
	void onIterStart(HMM& hmm, int iter);
	void onIterEnd(HMM& hmm, int iter, double score);
	void onUpdateStart(HMM& hmm, int iter);
	void onUpdateEnd(HMM& hmm, int iter);

private:
	void applyNesterov(HMM& hmm);
	void updateNesterov(HMM& hmm);

	std::vector<std::vector<double>> prevA;
	std::vector<std::vector<double>> prevB;
	std::vector<std::vector<double>> prevPi;
};




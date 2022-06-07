#pragma once

#include "HMM.h"

class Momentum;
class HMM;

/*


At the beginning of every epoch, this callback gets the updated learning rate value 
from schedule function provided at __init__, with the current epoch and current learning rate, 
and applies the updated learning rate on the optimizer.

At start of each iter, take iter and current momentum and set HMM momentum based on a function
*/



//Not the greatest design but I'm tired and low on time
class MomentumSchedule {
public:
	MomentumSchedule();

	virtual void onTrainStart(HMM& hmm, Momentum& m, int max_iters); //before 1st iter
	virtual void onTrainEnd(HMM& hmm, Momentum& m, int iter, double score);		//after last iter
	virtual void onIterStart(HMM& hmm, Momentum& m, int iter);	 //at start of each iter
	virtual void onIterEnd(HMM& hmm, Momentum& m, int iter, double score);	 //at end of each iter
	virtual void onUpdateStart(HMM& hmm, Momentum& m, int iter); //after????? calc gammas, before reestimate
	virtual void onUpdateEnd(HMM& hmm, Momentum& m, int iter); //after reestimate

	//TODO: Make protected?
	//Momentum* _momentum;
	double baseMomentum = 0;
	double currentMomentum = 0;
};


class SkipNSchedule : public MomentumSchedule {
public:
	SkipNSchedule(int iters);

	virtual void onIterStart(HMM& hmm, Momentum& m, int iter);	 //at start of each iter

	int num_skip_iters = 0;
};


class DisableSchedule : public MomentumSchedule {
public:
	DisableSchedule(int _start, int _end);

	virtual void onIterStart(HMM& hmm, Momentum& m, int iter);

	int start;
	int end;

};



class Skip1DisableSchedule : public MomentumSchedule {
public:
	Skip1DisableSchedule(int _start, int _end);

	virtual void onIterStart(HMM& hmm, Momentum& m, int iter);

	int start;
	int end;

};



class ReduceMomentumOnPlateau : public MomentumSchedule {
public:
	ReduceMomentumOnPlateau(int _patience, double _factor, int _delay);

	virtual void onIterStart(HMM& hmm, Momentum& m, int iter);
	virtual void onIterEnd(HMM& hmm, Momentum& m, int iter, double score);	 

	int delay;
	int patience;
	double factor;
	int curr_plateau_len = 0;
	double prev_score = -INFINITY;

};




class EntropySchedule : public MomentumSchedule {
public:
	EntropySchedule(double factor, double threshold);

	virtual void onIterEnd(HMM& hmm, Momentum& m, int iter, double score);

	double threshold;
	double factor;
	std::vector<double> history;


};


#include "schedule.h"

/*
TODO: Re-implement the old previously tested schedules with new structure
*/


MomentumSchedule::MomentumSchedule() {
	baseMomentum = 0;
	currentMomentum = 0;
}

void MomentumSchedule::onTrainStart(HMM& hmm, Momentum& m, int max_iters) {
	baseMomentum = m.getMomentum();
	currentMomentum = baseMomentum;
}

void MomentumSchedule::onTrainEnd(HMM& hmm, Momentum& m, int iter, double score) {
}

void MomentumSchedule::onIterStart(HMM& hmm, Momentum& m, int iter) {
}

void MomentumSchedule::onIterEnd(HMM& hmm, Momentum& m, int iter, double score) {
}

void MomentumSchedule::onUpdateStart(HMM& hmm, Momentum& m, int iter) {
}

void MomentumSchedule::onUpdateEnd(HMM& hmm, Momentum& m, int iter) {
}



SkipNSchedule::SkipNSchedule(int iters) {
	num_skip_iters = iters;
}

void SkipNSchedule::onIterStart(HMM& hmm, Momentum& m, int iter) {
	if (iter < num_skip_iters)
		m.setMomentum(0);
	else
		m.setMomentum(baseMomentum);
}


DisableSchedule::DisableSchedule(int _start, int _end) {
	start = _start;
	end = _end;
}

void DisableSchedule::onIterStart(HMM& hmm, Momentum& m, int iter) {
	if ((iter >= start && iter < end)) { // iter == 0 ||
		m.setMomentum(0);
		m.clear();
	}
	else
		m.setMomentum(baseMomentum);
}


Skip1DisableSchedule::Skip1DisableSchedule(int _start, int _end) {
	start = _start;
	end = _end;
}

void Skip1DisableSchedule::onIterStart(HMM& hmm, Momentum& m, int iter) {
	if (iter == 0 || (iter >= start && iter < end)) { 
		m.setMomentum(0);
		m.clear();
	}
	else
		m.setMomentum(baseMomentum);
}


ReduceMomentumOnPlateau::ReduceMomentumOnPlateau(int _patience, double _factor, int _delay) {
	delay = _delay;
	patience = _patience;
	factor = _factor;
	curr_plateau_len = 0;
	prev_score = -INFINITY;
}

void ReduceMomentumOnPlateau::onIterStart(HMM& hmm, Momentum& m, int iter) {
}

void ReduceMomentumOnPlateau::onIterEnd(HMM& hmm, Momentum& m, int iter, double score) {
	if (iter < delay) {
		return;
	}

	if (score > prev_score) {
		prev_score = score;
		curr_plateau_len = 0;
		return;
	}

	curr_plateau_len++;
	if (curr_plateau_len > patience) {
		m.setMomentum(m.getMomentum() * factor);
		curr_plateau_len = 0;
		std::cout << m.getMomentum() << std::endl;
	}
	 
	prev_score = score;
}






EntropySchedule::EntropySchedule(double _factor, double _threshold) {
	threshold = _threshold;
	factor = _factor;
}

void EntropySchedule::onIterEnd(HMM& hmm, Momentum& m, int iter, double score) {
	double entropy = hmm.s_entropy();
	history.push_back(entropy);
	
	if (history.size() < 3)
		return;

	int n = history.size() - 1;
	double dentropy = history[n] - history[n - 1];
	double dentropy2 = history[n - 1] - history[n - 2];

	double ddentropy = dentropy - dentropy2;
	if (ddentropy > 0.05 * history[n-1]) {
		m.setMomentum(m.getMomentum() * factor);
		std::cout << m.getMomentum() << std::endl;
	}
	else if (ddentropy < 0) {
		m.setMomentum(m.getMomentum() / factor);
		if (m.getMomentum() > baseMomentum)
			m.setMomentum(baseMomentum);
		std::cout << m.getMomentum() << std::endl;
	}


}

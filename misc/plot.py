"""

Plot mean, min, max scores during training given training history files, 
as well as the differences between values for two sets of scores.

"""

import matplotlib.pyplot as plt
import os
import sys
from os import listdir, makedirs
from os.path import isfile, join, isdir
from scipy import stats
import numpy as np
from math import sqrt

np.set_printoptions(suppress=True)



# Create the given directory if it doesn't already exist.
# Creates all folders in path that do not exist.
def init_dir(path):
    directory = os.path.dirname(path)
    if not os.path.isdir(directory): 
        os.makedirs(directory)
    


#Plot mean of each history in histories on a single plot
def plotHistories(histories, title, func='mean', iters=None, outfile=None, xlim=None, ylim=None, margin_factor=1.00, show=False): 
    descriptions = [h.description for h in histories]

    min_score = None
    max_score = None
    min_iter = None
    max_iter = None
 
    for history in histories:
        X = iters or list(range(history.iters))
        if func == 'mean':
            Y = history.mean()
        elif func == 'max':
            Y = history.max()
        elif func == 'min':
            Y = history.min()
        else:
            print("Specify type of plot")
            return
        Y = Y[X] #Only use desired iters
        
        plt.plot(X, Y)
        #https://stackoverflow.com/questions/22481854/plot-mean-and-standard-deviation
        #https://stackoverflow.com/questions/12957582/plot-yerr-xerr-as-shaded-region-rather-than-error-bars
        #plt.errorbar(x, y, e, linestyle='None', marker='^')
        
        min_iter = min(X) if min_iter is None else min(min_iter, min(X))
        max_iter = max(X) if max_iter is None else max(max_iter, max(X))
        min_score = min(Y) if min_score is None else min(min_score, min(Y))
        max_score = max(Y) if max_score is None else max(max_score, max(Y))

    #print(min_score)       
    #print(max_score)

    xlim = xlim or (min_iter,max_iter)
    #ylim = ylim or (min_score,max_score)

    plt.xlim(left=xlim[0])
    plt.xlim(right=xlim[1])
    if ylim is not None:
        plt.ylim(bottom=ylim[0] * margin_factor)    
        plt.ylim(top=ylim[1] / margin_factor)

    plt.title(title)
    plt.xlabel('Iter')
    plt.ylabel('Score')
    plt.legend(descriptions, loc='lower right') #loc='upper right'
    if outfile is not None:
        init_dir(outfile)
        plt.savefig(outfile, bbox_inches='tight', dpi=200)
    if show:
        plt.show()
    plt.close()




#plot difference between means of multiple histories, with 0 being baseline
def plotHistoryDiffs(baseline, histories, title, func='mean', iters=None, outfile=None, xlim=None, ylim=None, margin_factor=1.00, show=False): 
    descriptions = [h.description for h in histories]

    min_score = None
    max_score = None
    min_iter = None
    max_iter = None
    
    #plt.plot([],[])
    #descriptions = [None] + descriptions
    #fig, ax = plt.subplots()
    #ax._get_lines.get_next_color()
    #ax._get_lines.prop_cycler.__next__()
 
    for history in histories:
        X = iters or list(range(history.iters))
        if func == 'mean':
            Y = history.mean_diff(baseline)
        elif func == 'max':
            Y = history.max_diff(baseline)
        elif func == 'min':
            Y = history.min_diff(baseline)
        else:
            print("Specify type of plot")
            return
        Y = Y[X] #Only use desired iters
        
        plt.plot(X, Y)
        #https://stackoverflow.com/questions/22481854/plot-mean-and-standard-deviation
        #https://stackoverflow.com/questions/12957582/plot-yerr-xerr-as-shaded-region-rather-than-error-bars
        #plt.errorbar(x, y, e, linestyle='None', marker='^')
        
        min_iter = min(X) if min_iter is None else min(min_iter, min(X))
        max_iter = max(X) if max_iter is None else max(max_iter, max(X))
        min_score = min(Y) if min_score is None else min(min_score, min(Y))
        max_score = max(Y) if max_score is None else max(max_score, max(Y))

    #print(min_score)       
    #print(max_score)
    
    plt.axhline(y=0, linestyle='--', color='black')#, linewidth=4)

    xlim = xlim or (min_iter,max_iter)
    #ylim = ylim or (min_score,max_score)

    plt.xlim(left=xlim[0])
    plt.xlim(right=xlim[1])
    mf = margin_factor
    if ylim is not None:
        bot = ylim[0] * margin_factor if ylim[0] < 0 else 0
        plt.ylim(bottom=bot)
        top = ylim[1]
        top = top * mf if top > 0 else top / mf
        plt.ylim(top=top)
        #plt.ylim(bottom=ylim[0] * margin_factor)    
        #plt.ylim(top=ylim[1] / margin_factor)
        
        

    plt.title(title)
    plt.xlabel('Iter')
    plt.ylabel('Score')
    plt.legend(descriptions, loc='lower right')#, loc='lower right') #loc='upper right'
    if outfile is not None:
        init_dir(outfile)
        plt.savefig(outfile, bbox_inches='tight', dpi=200)
    if show:
        plt.show()
    plt.close()





#Plot score over time for each in list of single restart
def plotSingleRestarts(restarts, title, descriptions, iters=None, outfile=None, xlim=None, ylim=None, margin_factor=1.00, show=False):
    min_iter = None
    max_iter = None

    for restart in restarts:
        X = iters or list(range(len(restart)))
        Y = restart[X] #Only use desired iters
        plt.plot(X, Y)
        
        min_iter = min(X) if min_iter is None else min(min_iter, min(X))
        max_iter = max(X) if max_iter is None else max(max_iter, max(X))

    xlim = xlim or (min_iter,max_iter)
    #ylim = ylim or (min_score,max_score)

    plt.xlim(left=xlim[0])
    plt.xlim(right=xlim[1])
    if ylim is not None:
        plt.ylim(bottom=ylim[0] * margin_factor)    
        plt.ylim(top=ylim[1] * margin_factor)

    plt.title(title)
    plt.xlabel('Iter')
    plt.ylabel('Score')
    plt.legend(descriptions, loc='lower right') #loc='upper right'
    if outfile is not None:
        init_dir(outfile)
        plt.savefig(outfile, bbox_inches='tight', dpi=200)
    if show:
        plt.show()
    plt.close()    
    

    
# General plot each row in matrix
def plot(matrix, title, descriptions, output_file):
    pass
    
    


        
    #get_best, get_worst_score, etc?

#Histories for many HMM models restarts trained under certain conditions
class History():    #for many restarts of same parameters
    def __init__(self, N=None,M=None,T=None,m=None,s=None, description=""):
        self.histories = np.zeros(0)
        #self.restart_labels = []
        self.N = N
        self.M = M
        self.T = T
        self.momentum = m
        self.smoothing = s
        self.description = description
        self.iters = -1

    #  Maybe take list of parameters instead? not sure how format works
    def load(self, directory, file_format, restarts):
        filenames = []
        for restart in restarts:
            fname = directory + file_format.format(restart)
            filenames.append(fname)
        self.histories = np.vstack([np.loadtxt(f) for f in filenames])  #model.reshape(1,-1)
        self.iters = self.histories.shape[1]
        
    
    def compute_stats(self, restarts=None):
        return [self.compute_iter_stats(i) for i in range(self.histories.shape[1])]
        
    def compute_iter_stats(self, iteration, restarts=None):
        statistics = stats.describe(self.histories[:,iteration])
        
        results = {
            'mean' : statistics.mean,
            'stddev' : sqrt(statistics.variance),
            'min' : statistics.minmax[0],
            'max' : statistics.minmax[1],
            'skewness' : statistics.skewness,
            'kurtosis' : statistics.kurtosis
        }
        
        return results
        
    def mean(self, iters=None):
        return np.mean(self.histories, axis=0)
        
    def stddev(self, iters=None):
        return np.std(self.histories, axis=0, ddof=1)
        
    def min(self, iters=None):
        return np.amin(self.histories, axis=0)
        
    def max(self, iters=None):
        return np.amax(self.histories, axis=0)
        
    def diff(self, other):
        if self.histories.shape != other.histories.shape:
            print("Diff shapes do not match")
            return None
            
        delta = np.subtract(self.histories, other.histories)
        return delta
        
    def mean_diff(self, other):
        if self.histories.shape != other.histories.shape:
            print("Diff shapes do not match")
            return None
        #print(np.allclose(np.mean(self.histories - other.histories, axis=0), self.mean() - other.mean()))
        return np.mean(self.histories - other.histories, axis=0)

    #Diff of mins(difference between worsts)
    def min_diff(self, other):
        if self.histories.shape != other.histories.shape:
            print("Diff shapes do not match")
            return None
        return self.min() - other.min()

    #Diff of maxes(difference between bests)
    def max_diff(self, other):
        if self.histories.shape != other.histories.shape:
            print("Diff shapes do not match")
            return None        
        return self.max() - other.max()
 



def get_histories_minmax(histories, iters=None):
    min_score = min((np.min(h.histories[:,iters]) for h in histories))
    max_score = max((np.max(h.histories[:,iters]) for h in histories))
    return (min_score, max_score)









def test():
    data_dir = "output/text/test/examples/"
    output_dir = "results/plots/text/test/"
    
    if not os.path.isdir(data_dir):
        print("NO DATA FOUND")
        exit()    

    baseline = History(description="momentum=0.0")
    baseline.load(data_dir, "{:d}_0.txt", range(0,2))
    
    mom = History(description="momentum=0.9")
    mom.load(data_dir, "{:d}_1.txt", range(0,2))

    plotHistories([baseline, mom], "Mean", func='mean', iters=list(range(0,10)), outfile=output_dir+'mean.png', show=True)
    plotHistories([baseline, mom], "Max", func='max', iters=list(range(0,10)), outfile=output_dir+'max.png', show=True)
    plotHistories([baseline, mom], "Min", func='min', iters=list(range(0,10)), outfile=output_dir+'min.png', show=True)
    
    
    plotHistoryDiffs(baseline, [mom], "Mean", func='mean', iters=list(range(0,10)), outfile=output_dir+'mean.png', show=True)
    plotHistoryDiffs(baseline, [mom], "Max", func='max', iters=list(range(0,10)), outfile=output_dir+'max.png', show=True)
    plotHistoryDiffs(baseline, [mom], "Min", func='min', iters=list(range(0,10)), outfile=output_dir+'min.png', show=True)

 
 
 
#Compare stats between each session, baseline?
#Plot single histories in 1 plot
#Plot MEAN of list of sessions and baseline in 1 plot
#Plot differences between each session and baseline 
def main():
    N = 27
    M = 27
    T = 1000
    momentum = 0.5
    nesterov = True
    start_iter = 0
    end_iter = 500
    first_restart = 0
    last_restart = 100
    plot = False
    families = ['winwebsec', 'zbot', 'zeroaccess']
    
    description = ""
    filename = "{:d}_hmmM_{:.6f}.csv"
    data_dir = "results/text/report/plateau T=10000/"
    output_dir = "results/plots/text/test/"
    
    if not os.path.isdir(data_dir):
        print("NO DATA FOUND")
        exit()

    baseline = History(description="momentum=0.0")
    #baseline.load(data_dir + "T=1000 N=27 M=27 m=0.5 Nesterov=1 smooth=0 SKIP 1" + '/', "{:d}_hmmM_0.000000.csv", range(first_restart, last_restart))
    #baseline.load(data_dir + "T=10000 N=27 M=27 m=0.5 Nesterov=1 smooth=0 FIXED PI/", "{:d}_hmmM_0.000000.csv", range(first_restart, last_restart))

    mom = History(description="momentum=0.9")
    #mom.load(data_dir + "T=1000 N=27 M=27 m=0.5 Nesterov=1 smooth=0 SKIP 1/", "{:d}_hmmM_0.500000.csv", range(first_restart, last_restart))
    #mom.load(data_dir + "T=10000 N=27 M=27 m=0.5 Nesterov=1 smooth=0 FIXED PI/", "{:d}_hmmM_0.500000.csv", range(first_restart, last_restart))
    
    #nesterov = History(description="nesterov=0.5")
    #nesterov.load(data_dir + "T=10000 N=27 M=27 Nesterov=1 divby10 1 to 50/", "{:d}_hmmM_0.500000.csv", range(first_restart, last_restart))
    
    #iter_stats = baseline.compute_stats()
    
    histories = [baseline, mom]
    iters = list(range(1,500))
    mf=1.01
    xlim = (iters[0], iters[-1])
    #ylim = get_histories_minmax(histories, list(range(xlim[0],xlim[1])))
    ylim = get_histories_minmax(histories, iters)
    
    plotHistories([baseline, mom], "Mean", func='mean', iters=iters, outfile=output_dir+'mean.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    plotHistories([baseline, mom], "Max", func='max', iters=iters, outfile=output_dir+'max.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    plotHistories([baseline, mom], "Min", func='min', iters=iters, outfile=output_dir+'min.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    
    
    min_score = float('inf')
    max_score = float('-inf')
    
    for h in histories[1:]:
        min_score = min(min_score, np.min(h.mean_diff(baseline)))
        min_score = min(min_score, np.min(h.min_diff(baseline)))
        min_score = min(min_score, np.min(h.max_diff(baseline)))
        
        max_score = max(max_score, np.max(h.mean_diff(baseline)))
        max_score = max(max_score, np.max(h.min_diff(baseline)))
        max_score = max(max_score, np.max(h.max_diff(baseline)))
        
    ylim = (min_score,max_score)
    
    mf=1.0

    plotHistoryDiffs(baseline, [mom], "Mean score difference with momentum", func='mean', iters=iters, outfile=output_dir+'meandiff.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    plotHistoryDiffs(baseline, [mom], "Difference in max score from adding momentum", func='max', iters=iters, outfile=output_dir+'maxdiff.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    plotHistoryDiffs(baseline, [mom], "Difference in min score from adding momentum", func='min', iters=iters, outfile=output_dir+'mindiff.png', show=True, xlim=xlim, ylim=ylim, margin_factor=mf)
    
    '''
    plotHistories([baseline, mom], "Mean", func='mean', iters=iters, outfile=output_dir+'mean.png', show=True)
    plotHistories([baseline, mom], "Max", func='max', iters=iters, outfile=output_dir+'max.png', show=True)
    plotHistories([baseline, mom], "Min", func='min', iters=iters, outfile=output_dir+'min.png', show=True)
    
    plotHistoryDiffs(baseline, [mom], "Mean score difference with momentum", func='mean', iters=iters, outfile=output_dir+'meandiff.png', show=True)
    plotHistoryDiffs(baseline, [mom], "Difference in max score from adding momentum", func='max', iters=iters, outfile=output_dir+'maxdiff.png', show=True)
    plotHistoryDiffs(baseline, [mom], "Difference in min score from adding momentum", func='min', iters=iters, outfile=output_dir+'mindiff.png', show=True)
    '''

    restart0 = [h.histories[0] for h in histories]
    descriptions = [h.description for h in histories]
    plotSingleRestarts(restart0, "Restart 0", descriptions, outfile=output_dir+'single.png', show=True)
    
    #print(mom.mean_diff(baseline))
    





if __name__ == "__main__":
    #test()
    main()

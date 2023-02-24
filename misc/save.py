#import numpy as np

#numpy.savetxt(fname, X, delimiter=' ', header='')
def save_csv(vectors, filename, headers=[]):       
    if len(vectors) == 0:
        return
        
    with open(filename, 'w') as f:
        if len(headers) > 0:
            for h in headers[:-1]:
                f.write(h + ',')
            f.write(headers[-1] + '\n')
        
        num_entries = min((len(v) for v in vectors))
        for i in range(num_entries):
            for vec in vectors[:-1]:
                f.write(str(vec[i]) + ',')
            f.write(str(vectors[-1][i]) + '\n')
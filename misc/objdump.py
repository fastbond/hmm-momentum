import subprocess
from os import listdir
from os.path import isfile, join
import os
#from os import walk
import re


# Need to check fname validity(does file exist, is fname not None/empty)
def dump_file(fname, outfile=None):
    # TODO: check if file exists
    if fname is None or fname == "": 
        return
    
    if outfile is None:
        outfile = fname + ".txt"

    cmd = "objdump -d " + fname
    p = subprocess.run(cmd.split(), capture_output=True) #stdout=subprocess.PIPE, stderr=subprocess.STDOUT) #capture_output=True, 

    output = p.stdout.decode()
    output = output.replace('\r', '')
    
    opcodes = []
    # For each line, opcode = text after 2nd tab, before any whitespace/eof(before spaces or newline or eof)
    pattern = re.compile("[^\t]+\t[^\t]+\t(\S+).*")
    for line in output.splitlines():
        match = pattern.fullmatch(line)
        if match is not None:
            opcode = match.group(1)
            opcodes.append(opcode + '\n')

    if len(opcodes) != 0:
        with open(outfile, "w") as f:
            f.writelines(opcodes)
            #f.write(output)   
        
        

def list_files(directory):
    #https://stackoverflow.com/questions/3207219/how-do-i-list-all-files-of-a-directory
    files = [join(directory, f) for f in listdir(directory) if isfile(join(directory, f))]
    return files
        


def read_file(fname):
    lines = []
    with open(fname, 'r') as f:
        for line in f:
            line = line.strip()
            if line == "":
                continue
            lines.append(line)
    return lines
    
        
def init_dir(path):
    directory = os.path.dirname(path)
    if not os.path.isdir(directory):
        os.makedirs(directory)
        
        
        
def main():
    exe_dir = '/home/meh/Desktop/malware/exe/'
    file_list_dir = '/home/meh/Desktop/malware/malware_files/'
    output_dir = '/home/meh/Desktop/malware/objdump/'
    families = ['VBInject', 'Winwebsec', 'Renos', 'OnLineGames', 'BHO', 'Startpage', 'Adload', 'VB', 'Vobfus']
    
    success = 0
    fail = 0
    for family in families:
        print(family)
        init_dir(output_dir + family + '/')
        
        list_file = file_list_dir + family + '.txt'
        samples = read_file(list_file)   
        for sample in samples:
            try:
                dump_file(exe_dir + family + '/' + sample, output_dir + family + '/' + sample + '.txt')
                success += 1
            except Exception as e:
                print(sample)
                print(e)
                fail += 1
        print()
    print("Success: " + str(success))
    print("Failed: " + str(fail))
    
    
    '''
    #Load list of files from files.txt
    #For each file, dump_file to file of same name.txt in specified output directory
    with open(listfile,'r') as f:
        for line in f:
            line = line.strip()
            if line == "":
                continue
            sample_fname = input_dir + line
            output_fname = output_dir + line + '.txt'
            try:
                dump_file(sample_fname, output_fname) 
            except Exception as e:
                print(e)
    #need to test objdump comparison linux vs. windows
    #no diff!            
    '''





if __name__ == "__main__":
    main()


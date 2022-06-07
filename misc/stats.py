import os
import matplotlib.pyplot as plt

from save import save_csv




def get_families(list_file):
    families = []
    with open(list_file, 'r') as f:
        families = [line.strip() for line in f if line.strip != '']
    return families
    
    
def sum_dicts(dicts):
    totals = {}
    for d in dicts:
        for k,v in d.items():
            if k not in totals:
                totals[k] = 0
            totals[k] += v
    return totals
    
    
def count_opcodes(opcode_counts_list):
    total = 0
    for opcode_count_map in opcode_counts_list:
        for k,v in opcode_count_map.items():
            total += v
    return total
    

    

def malware_opcodes():
    data_dir = "data/malware/"
    family_file = "test_families.txt"
    top_n_families = 15
    M = 30
    num_top_opcodes = M-1
    
    families = get_families(family_file)[:top_n_families]
    
    counts = {}
    for family in families:
        print(family)
        files = os.listdir(data_dir + family)
        counts[family] = {}
        for file in files:
            with open(data_dir + family + '/' + file, 'r') as f:
                for line in f:
                    opcode = line.strip()
                    if opcode == '':
                        continue
                    if opcode not in counts[family]:
                        counts[family][opcode] = 0
                    counts[family][opcode] += 1
        

    totals = sum_dicts(counts.values())
    totals = {k: v for k, v in sorted(totals.items(), key=lambda item: item[1], reverse=True)}
    
    top_opcodes = list(totals.keys())[:num_top_opcodes]
    print("Top M-1 opcodes: ")
    print(top_opcodes)
    print()
    
    top_count = sum((v for k,v in totals.items() if k in top_opcodes))
    print("Total number of opcodes: " + str(count_opcodes([totals])))
    print("Total number of opcodes in top M-1: " + str(top_count))
    print("Percent: " + str(top_count / count_opcodes([totals]) * 100))
    print()
    
    families = sorted(families)
    
    top_pcts = []
    for family in families:
        print(family)
        total = count_opcodes([counts[family]])
        top = sum((v for k,v in counts[family].items() if k in top_opcodes))
        print(top / total * 100)
        top_pcts.append(top / total)
    
    fig, ax = plt.subplots(figsize=(10,6))
    plt.bar(families, top_pcts)
    plt.xticks(rotation=45, ha="center")
    ax.ticklabel_format(useOffset=False, style='plain', axis='y')
    plt.ylim(0,1)
    
    plt.xlabel("Families")
    plt.ylabel("Top Opcode Percentage")
    #plt.title("")
    
    plt.savefig('results/final/malware/plots/top_opcode_percent.png', dpi=200, bbox_inches='tight')
    #plt.show()
    plt.close()

    
    # Now can easily find counts and therefore percents for each family of these top opcodes
    # And overall
    # Then print/plot
    # count(topN) / familyTotal for each family
    # count(topN) / total for all
    
    
    #https://www.geeksforgeeks.org/bar-plot-in-matplotlib/
    files_per_family = [len(os.listdir(data_dir + family)) for family in families]
    fig, ax = plt.subplots(figsize=(10,6))
    plt.bar(families, files_per_family)
    plt.xticks(rotation=45, ha="center")
    ax.ticklabel_format(useOffset=False, style='plain', axis='y')
    
    plt.xlabel("Families")
    plt.ylabel("Number of Files")
    #plt.title("")
    
    plt.savefig('results/final/malware/plots/family_files.png', dpi=200, bbox_inches='tight')
    #plt.show()
    plt.close()
    
    
    
    #fig = plt.figure()
    #plt.gca()
    fig, ax = plt.subplots(figsize=(10,6))
 
    # creating the bar plot
    values = [count_opcodes([counts[family]]) for family in families]
    plt.bar(families, values)
    
    plt.xticks(rotation=45, ha="center")
    ax.ticklabel_format(useOffset=False, style='plain', axis='y')
    
    plt.xlabel("Families")
    plt.ylabel("Total Opcodes")
    #plt.title("")
    
    plt.savefig('results/final/malware/plots/family_opcodes.png', dpi=200, bbox_inches='tight')
    #plt.show()
    plt.close()
    
    
    fig, ax = plt.subplots(figsize=(10,6))
 
    # creating the bar plot
    values = [count_opcodes([counts[family]]) / count_opcodes([totals]) for family in families]
    print(sum(values))
    print(values)
    plt.bar(families, values)
    
    plt.xticks(rotation=45, ha="center")
    ax.ticklabel_format(useOffset=False, style='plain', axis='y')
    plt.ylim(0,1)
    
    plt.xlabel("Families")
    plt.ylabel("Total Opcodes")
    #plt.title("")
    
    plt.savefig('results/final/malware/plots/family_opcodes_percents.png', dpi=200, bbox_inches='tight')
    plt.close()
    
    
    
    values = [cnt / count_opcodes([totals]) for opc, cnt in totals.items()]#[:num_top_opcodes]
    other = sum(values[num_top_opcodes:])
    values = values[:num_top_opcodes]
    print(totals)
    labels = [opc for opc, cnt in totals.items()][:num_top_opcodes] + ['Other']
    print(labels)
    values += [(count_opcodes([totals]) - top_count) / count_opcodes([totals])]
    print(other)
    print(values[-1])
    plt.bar(labels, values)
    
    #plt.xticks(rotation=45, ha="center")
    plt.xticks(rotation='vertical')
    ax.ticklabel_format(useOffset=False, style='plain', axis='y')
    
    plt.xlabel("Opcodes")
    plt.ylabel("Percent")
    #plt.title("")
    
    plt.savefig('results/final/malware/plots/opcode_distribution.png', dpi=200, bbox_inches='tight')
    plt.show()
    plt.close()
    
    #output_dir = 'results/final/malware/plots/'
    #save_csv([families, top_pcts], output_dir + 'top_opcode_percent.csv', headers=['Families', 'Top 29 Pct']) 
    #save_csv([families, files_per_family], output_dir + 'family_files.csv', headers=['Families', 'Files']) 
    #save_csv([labels, values], output_dir + 'opcode_distribution.csv', headers=['Families', 'Opcode Counts']) 
    


def main():
    malware_opcodes()
    

if __name__ == '__main__':
    main()
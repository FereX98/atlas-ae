import numpy as np
import sys

def process(in_file, out_file):
    src_v = {}
    
    with open(in_file, 'r') as f:
        lines = f.readlines()
        lines = lines[1:]
        if lines[-1].strip() == '':
            lines = lines[:-1]
        edges = np.array(list(map(lambda x: [int(x.split()[0]), int(x.split()[1])], lines)), dtype=int)
    n = np.max(edges)
    m = edges.shape[0]
    
    print("The input graph has {} vertices and {} edges".format(n, m)) 

    for i in range(m):
        src,dst = edges[i]
        if src not in src_v:
            src_v[src] = [dst]
        else:
            src_v[src].append(dst)

    output = ['AdjacencyGraph', str(n + 1), str(m), '0']
    last_offset = 0

    for i in range(1, n + 1): 
        output.append(str(last_offset))
        if i not in src_v:
            continue
        last_offset += len(src_v[i])
    for i in range(1, n + 1): 
        i = int(i)
        if i not in src_v:
            continue
        ss = '\n'.join(map(str, src_v[i]))
        output.append(ss)
    
    with open(out_file, 'w') as f:
        f.write('\n'.join(output))

    print("Generated file: {}".format(out_file)) 

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage : python3 {} <in_file> <out_file>".format(sys.argv[0]))
        exit(1)
    process(sys.argv[1], sys.argv[2])

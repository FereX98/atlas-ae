import sys

def find_and_write(input_file, output_file):
    # Open the input file
    with open(input_file, 'r') as f, open(output_file, 'w') as out_file:
        # Iterate over each line in the file
        Start = False
        i = 0
        for line in f:
            if Start:
                out_file.write(line)
                i += 1
                if i == 99:
                    return
            elif line.strip().startswith("percentiles:"):
                Start = True

if __name__ == "__main__":
    input_file = f"/home/osdi/ae/results/fig6b/atlas.log"
    output_file = f"/home/osdi/ae/results/fig6b/atlas.csv"
    find_and_write(input_file, output_file)

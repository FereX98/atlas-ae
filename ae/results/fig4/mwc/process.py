import re
import sys
import os

def parse_results(input_file, output_file):
    pattern = re.compile(r"total time (\d+) ms")
    with open(input_file, 'r') as f:
        # Iterate over each line in the file
        for line in f:
            line = line.strip()
            if pattern.match(line):
                # Extract the number from the line
                number = int(pattern.match(line).group(1))
                # Write the number into the output file
                with open(output_file, 'w') as out_file:
                    out_file.write(str(number))
                return  # Exit the function after finding and writing the number
    print(f"Input file {input_file} is broken!")
    sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python process.py <ratio>")
        sys.exit(1)
    ratio = sys.argv[1]
    if int(ratio) not in [13, 25, 50, 75, 100]:
        print(f"Invalid ratio: {ratio}, must be one of [13|25|50|75|100]")
        sys.exit(1)
    input_file = f"/home/osdi/ae/results/fig4/mwc/atlas-{ratio}-raw"
    if not os.path.exists(input_file):
        print(f"Input file {input_file} does not exist")
        sys.exit(1)
    output_file = f"/home/osdi/ae/results/fig4/mwc/atlas-{ratio}"
    parse_results(input_file, output_file)

import sys

def find_and_write_third_term(input_file, output_file):
    # Open the input file
    with open(input_file, 'r') as f:
        # Iterate over each line in the file
        for line in f:
            # Check if the line starts with "zero"
            if line.strip().startswith("Total:"):
                # Split the line by ","
                parts = line.strip().split()
                # Check if there are at least 3 terms
                if len(parts) >= 2:
                    third_term = parts[1]
                    # Write the third term into the output file
                    with open(output_file, 'w') as out_file:
                        out_file.write(third_term)
                    return  # Exit the function after finding and writing the third term

if __name__ == "__main__":
    ratio = sys.argv[1]
    input_file = f"/home/osdi/ae/results/fig4/df/atlas-{ratio}-raw"
    output_file = f"/home/osdi/ae/results/fig4/df/atlas-{ratio}"
    find_and_write_third_term(input_file, output_file)

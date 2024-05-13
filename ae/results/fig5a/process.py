if __name__ == "__main__":
    ratios = [0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55]
    path = "/home/osdi/ae/results/fig5a"
    with open(f"{path}/atlas-t", "w") as t, open(f"{path}/atlas-l", "w") as l:
        for ratio in ratios:
            input_file = f"{path}/atlas-{ratio}"
            with open(input_file, 'r') as f:
                # Iterate over each line in the file
                for line in f:
                    # Check if the line starts with "zero"
                    if line.startswith("final mops ="):
                        parts = line.split()
                        t.write(parts[-1])
                        t.write("\n")
                    elif line.startswith("90 tail lat (cycles) ="):
                        parts = line.split()
                        l.write(parts[-1])
                        l.write("\n")

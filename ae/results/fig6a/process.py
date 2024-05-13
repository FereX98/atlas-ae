if __name__ == "__main__":
    throughputs = [0.1, 0.2, 0.3, 0.4, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.4, 4.6]
    path = "/home/osdi/ae/results/fig6a"
    with open(f"{path}/atlas-t", "w") as t, open(f"{path}/atlas-l", "w") as l:
        for throughput in throughputs:
            input_file = f"{path}/atlas-{throughput}"
            with open(input_file, 'r') as f:
                # Iterate over each line in the file
                for line in f:
                    # Check if the line starts with "zero"
                    if line.startswith("zero"):
                        parts = line.split(", ")
                        t.write(parts[2])
                        t.write("\n")
                        l.write(parts[6])
                        l.write("\n")
                        break

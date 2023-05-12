import matplotlib.pyplot as plt
import matplotlib
import numpy as np

nthreads = ["(1,1)", "(1,2)", "(1,4)", "(1,8)", "(2,1)", "(4,1)", "(8,1)"]
results = []

filename = "results.txt"

with open(filename, "r") as file:
    lines = file.readlines()

for line in lines:
    ints = [float(x) for x in line.split()]
    results.append(ints)

plt.figure(figsize=(12, 8))
plt.plot([k for k in range(len(nthreads))], results[0], 'o-', label='1')
plt.plot([k for k in range(len(nthreads))], results[1], 'o-', label='10')
plt.plot([k for k in range(len(nthreads))], results[2], 'o-', label='100')
plt.plot([k for k in range(len(nthreads))], results[3], 'o-', label='1000')
plt.xticks([k for k in range(len(nthreads))], nthreads)
plt.tick_params('x', labelsize = 6)

plt.xlabel('Number of Productor Threads (Np) and Consumer Threads (Nc) - (Np, Nc)')
plt.ylabel('Time (s)')
plt.title('Runtime for productor and consumer', weight = 'bold')
plt.legend()
plt.savefig("results_produtor-consumidor-time.png")
plt.show()
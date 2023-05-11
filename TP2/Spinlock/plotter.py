import matplotlib.pyplot as plt
import matplotlib
import numpy as np

nthreads = [1,2,4,8,16,32,64,128,256]
results = []

filename = "TP2/Spinlock/results.txt"

with open(filename, "r") as file:
    lines = file.readlines()

for line in lines:
    ints = [float(x) for x in line.split()]
    results.append(ints)

plt.figure(figsize=(12, 8))
plt.plot(nthreads, results[0], 'o-', label='10^7')
plt.plot(nthreads, results[1], 'o-', label='10^8')
plt.plot(nthreads, results[2], 'o-', label='10^9')
plt.xticks(nthreads)
plt.tick_params('x', labelsize = 6)
for N in results:
    plt.plot(nthreads[N.index(min(N))], min(N), 'o-', color = 'r')

plt.xlabel('Number of Threads')
plt.ylabel('Time (ms)')
plt.title('Runtime for sum of elements in an array', weight = 'bold')
plt.legend()
plt.savefig("TP2/Spinlock/results_spinlock.png")
plt.show()
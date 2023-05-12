import matplotlib.pyplot as plt
import matplotlib
import numpy as np


arrayN = [1, 10, 100, 1000]
arrayNpNc = [[1,1], [1,2], [1,4], [1,8], [2,1], [4,1], [8,1]]

for N in range(len(arrayN)):
    for th in range(len(arrayNpNc)):
        results = []
        filename = "ocupation-txt/ocupation_"+str(arrayN[N])+"_"+str(arrayNpNc[th][0])+"_"+str(arrayNpNc[th][1])+".txt"

        with open(filename, "r") as file:
            lines = file.readlines()

        for line in lines:
            ints = [int(x) for x in line.split()]
            results.append(ints)

        plt.figure(figsize=(12, 8))
        if (N ==0):
            plt.plot([k+1 for k in range(len(results[0][:50]))], results[0][:50], 'o-')
            plt.xticks([k for k in range(0,len(results[0][:50]))])
        else:
            plt.plot([k+1 for k in range(len(results[0][:arrayN[N]*50]))], results[0][:arrayN[N]*50], 'o-')
            plt.xticks([k for k in range(0,len(results[0][:arrayN[N]*50]), (5*pow(10,N)))])
        plt.tick_params('x', labelsize = 6)

        plt.xlabel('Number of Operations')
        plt.ylabel('Buffer Ocupation Size')
        plt.title('Number of Ocupation in Buffer - N='+str(N)+' | Np='+str(arrayNpNc[th][0])+' | Nc='+str(arrayNpNc[th][1])+' - [0, '+str(50*pow(10,N))+']', weight = 'bold')
        plt.legend()
        plt.savefig("ocupation-img/ocupation_"+str(arrayN[N])+"_"+str(arrayNpNc[th][0])+"_"+str(arrayNpNc[th][1])+".png")
        # plt.show()
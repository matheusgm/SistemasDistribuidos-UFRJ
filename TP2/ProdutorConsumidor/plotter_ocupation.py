import matplotlib.pyplot as plt
import matplotlib
import numpy as np


arrayN = [1, 10, 100, 1000]
arrayNpNc = [[1,1], [1,2], [1,4], [1,8], [2,1], [4,1], [8,1]]
fig = plt.figure(figsize=(15, 21))

for N in range(len(arrayN)):
    for th in range(len(arrayNpNc)):
        results = []
        filename = "ocupation-txt/ocupation_"+str(arrayN[N])+"_"+str(arrayNpNc[th][0])+"_"+str(arrayNpNc[th][1])+".txt"

        with open(filename, "r") as file:
            lines = file.readlines()

        for line in lines:
            ints = [int(x) for x in line.split()]
            results.append(ints)

        plt.subplot(7, 4, N+1+ 4*th)
        if (N ==0):
            plt.plot([k+1 for k in range(len(results[0][:50]))], results[0][:50], 'o-')
            plt.xticks(np.arange(0,len(results[0][:50]),2))
        else:
            plt.plot([k+1 for k in range(len(results[0][:arrayN[N]*50]))], results[0][:arrayN[N]*50], 'o-')
            plt.xticks(np.arange(0,len(results[0][:arrayN[N]*50]), (5*pow(10,N))))
        plt.tick_params('x', labelsize = 6)

        plt.title('Np='+str(arrayNpNc[th][0])+' | Nc='+str(arrayNpNc[th][1])+' - [0, '+str(50*pow(10,N))+']', weight = 'bold')
        # plt.show()   

plt.tight_layout(rect=[0.04, 0.025, 1, 0.95])
fig.supxlabel('Number of Operations', fontsize = 'xx-large')
fig.supylabel('Buffer Ocupation Size', fontsize = 'xx-large')
fig.suptitle('Shared Memory Occupation Result', fontsize = 'xx-large', weight = 'bold')
plt.savefig("ocupation.png")
from glob import glob
from collections import OrderedDict

runs = OrderedDict()
runs["OneLockQueue"] = []
runs["TwoLockQueue"] = []
runs["NonBlockingQueue"] = []

def importantLines(filename):
    block=[]
    startReading = False
    with open(filename, "r") as slurmFile:
        for line in slurmFile:
            if startReading:
                block.append(line.strip())

            if "---" in line:
                startReading = True

            if "===" in line:
                startReading = False
                yield block
                block = []

def blockFilter(block):
    queueType = ""
    values = []
    for item in block:
        if "Queue" in item:
            queueType = item.split()[1]
        if "Total throughput =" in item:
            throughput = int(item.split()[-1])
            throughputList = runs[queueType]
            throughputList.append(throughput)
            runs[queueType] = throughputList

def findAverage():
    print("Type of Queue".center(20, " ") + "|" + "Throughput".center(20, " "))
    print("-"*41)
    for queueType, throughputList in runs.items():
        mean = sum(throughputList)/len(throughputList)
        print(queueType.center(20, " ") + "|" + str(mean).center(20, " "))

def main():
    successfulCount = 0
    totalFiles = 0

    for filename in glob("runs/slurm-*.out"):
        totalFiles += 1
        with open(filename, "r") as slurmFile:
            for line in slurmFile:
                if "Verification successful" in line:
                    successfulCount += 1
    for filename in glob("runs/slurm-*.out"):
        for block in importantLines(filename):
            blockFilter(block)
    
    print("Asserting if all runs were successful...")
    assert(totalFiles == successfulCount / 3)
    print("\nActual Total Number of Files: " + str(totalFiles))
    print("Successful runs: " + str(successfulCount) + "\nTotal Number of Files: " + str(successfulCount/3) + "\n")
    findAverage()
    print("")

if __name__ == "__main__":
    main()
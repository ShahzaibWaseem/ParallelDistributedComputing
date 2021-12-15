#!/bin/bash
#
#SBATCH --cpus-per-task=1
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=slow

echo -e "Calculate the mean throughputs from the slurm files in the runs/ directory\n"
srun python /home/$USER/assignment5/average.py
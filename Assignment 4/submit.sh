#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=05:00
#SBATCH --mem=5G
#SBATCH --partition=slow

# srun /home/$USER/assignment4/triangle_counting_parallel --nWorkers 4 --inputFile /scratch/input_graphs/lj

# srun /home/$USER/assignment4/page_rank_parallel --nWorkers 4 --granularity 2000 --nIterations 20 --inputFile /scratch/input_graphs/lj
# echo
# srun /home/$USER/assignment4/page_rank_parallel --nWorkers 4 --granularity 1 --nIterations 20 --inputFile /scratch/input_graphs/lj

srun python /scratch/assignment4/test_scripts/page_rank_tester.pyc --execPath=/home/msw10/assignment4/page_rank_parallel

# srun python /scratch/assignment4/test_scripts/submission_validator.pyc --tarPath=/home/msw10/assignment4/assignment4.tar.gz
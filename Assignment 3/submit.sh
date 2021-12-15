#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=slow

# srun /home/$USER/assignment3/page_rank_parallel --nWorkers 4 --strategy 1 --nIterations 20 --inputFile /scratch/input_graphs/lj
# echo
# echo
# srun /home/$USER/assignment3/page_rank_parallel --nWorkers 4 --strategy 2 --nIterations 20 --inputFile /scratch/input_graphs/lj

# srun python /scratch/assignment3/test_scripts/page_rank_tester.pyc --execPath=/home/msw10/assignment3/page_rank_parallel

srun python /scratch/assignment3/test_scripts/submission_validator.pyc --tarPath=/home/msw10/assignment3/assignment3.tar.gz
#!/bin/bash
#
#SBATCH --cpus-per-task=1
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --mem=10G

# srun /home/$USER/assignment6/triangle_counting_parallel
# python /scratch/assignment6/test_scripts/triangle_counting_tester.pyc --execPath=/home/$USER/assignment6/triangle_counting_parallel

# srun /home/$USER/assignment6/page_rank_parallel
# python /scratch/assignment6/test_scripts/page_rank_tester.pyc --execPath=/home/$USER/assignment6/page_rank_parallel

python /scratch/assignment6/test_scripts/submission_validator.pyc --tarPath=/home/$USER/assignment6/assignment6.tar.gz
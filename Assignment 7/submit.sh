#!/bin/bash
#
#SBATCH --cpus-per-task=1
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --mem=10G

# srun /home/$USER/assignment7/triangle_counting_parallel --strategy 1
# echo
# srun /home/$USER/assignment7/triangle_counting_parallel --strategy 2
# python /scratch/assignment7/test_scripts/triangle_counting_tester.pyc --execPath=/home/$USER/assignment7/triangle_counting_parallel

srun /home/$USER/assignment7/page_rank_parallel --strategy 1
echo
srun /home/$USER/assignment7/page_rank_parallel --strategy 2
# python /scratch/assignment7/test_scripts/page_rank_tester.pyc --execPath=/home/$USER/assignment7/page_rank_parallel

# python /scratch/assignment7/test_scripts/submission_validator.pyc --tarPath=/home/$USER/assignment7/assignment7.tar.gz
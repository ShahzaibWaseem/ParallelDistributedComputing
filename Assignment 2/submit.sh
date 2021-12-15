#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=fast

# echo "Single Threaded"
# echo
# srun /home/$USER/assignment2/page_rank_parallel --nWorkers 1
# echo
# echo "Multi Threaded"
# echo
# srun /home/$USER/assignment2/page_rank_parallel --nWorkers 4
# echo
# echo "Testing"
# echo
# srun python /scratch/assignment2/test_scripts/page_rank_lock_tester.pyc --execPath=/home/$USER/assignment2/page_rank_parallel
# echo
# echo "Atomic Single Threaded"
# echo
# srun /home/$USER/assignment2/page_rank_parallel_atomic --nWorkers 1
# echo
# echo "Atomic Multi Threaded"
# echo
# srun /home/$USER/assignment2/page_rank_parallel_atomic --nWorkers 4
# echo
# echo "Testing"
# echo
# srun python /scratch/assignment2/test_scripts/page_rank_atomic_tester.pyc --execPath=/home/$USER/assignment2/page_rank_parallel_atomic
# srun python /scratch/assignment2/test_scripts/submission_validator.pyc --tarPath=/home/$USER/assignment2/assignment2.tar.gz
#!/bin/bash
#
#SBATCH --cpus-per-task=8
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=fast

echo "OneLockQueue"
echo "Correctness"
srun /home/$USER/assignment5/one_lock_queue_correctness --n_producers 4 --n_consumers 4 --init_allocator 100000000
echo "-------------------------------------------------------"
echo "Throughput"
srun /home/$USER/assignment5/one_lock_queue_throughput --n_producers 4 --n_consumers 4 --seconds 5 --init_allocator 100000000
echo -e "\n=======================================================\n"

echo "TwoLockQueue"
echo "Correctness"
srun /home/$USER/assignment5/two_lock_queue_correctness --n_producers 4 --n_consumers 4 --init_allocator 100000000
echo "-------------------------------------------------------"
echo "Throughput"
srun /home/$USER/assignment5/two_lock_queue_throughput --n_producers 4 --n_consumers 4 --seconds 5 --init_allocator 100000000
echo -e "\n=======================================================\n"

echo "NonBlockingQueue"
echo "Correctness"
srun /home/$USER/assignment5/non_blocking_queue_correctness --n_producers 4 --n_consumers 4 --init_allocator 100000000
echo "-------------------------------------------------------"
echo "Throughput"
srun /home/$USER/assignment5/non_blocking_queue_throughput --n_producers 4 --n_consumers 4 --seconds 5 --init_allocator 100000000
echo -e "\n======================================================="

# srun python /scratch/assignment5/test_scripts/submission_validator.pyc --tarPath=/home/msw10/assignment5/assignment5.tar.gz
#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=02:00
#SBATCH --mem=5G
#SBATCH --partition=fast

echo -e "Pi Calculation\n"
srun /home/$USER/assignment1/pi_calculation_parallel --nWorkers 1 --nPoints 123456789
echo
srun /home/$USER/assignment1/pi_calculation_parallel --nWorkers 4 --nPoints 123456789
echo -e "\nPi Calculation Validation\n"
srun python /scratch/assignment1/test_scripts/pi_calculation_tester.pyc --execPath=/home/msw10/assignment1/pi_calculation_parallel
echo -e "\nTriangle Counting\n"
srun /home/$USER/assignment1/triangle_counting_parallel --nWorkers 1 --inputFile /scratch/input_graphs/lj
echo
srun /home/$USER/assignment1/triangle_counting_parallel --nWorkers 4 --inputFile /scratch/input_graphs/lj
echo
srun /home/$USER/assignment1/triangle_counting_parallel --nWorkers 1 --inputFile /scratch/input_graphs/test_25M_50M
echo
srun /home/$USER/assignment1/triangle_counting_parallel --nWorkers 4 --inputFile /scratch/input_graphs/test_25M_50M
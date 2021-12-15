echo "Cleaning"
make clean
echo "Making"
make page_rank_parallel
sbatch submit.sh
echo
squeue
echo
echo "Output"
echo
sleep 4
code `ls -t | head -1`
cat `ls -t | head -1`
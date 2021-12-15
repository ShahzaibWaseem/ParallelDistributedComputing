echo "Cleaning"
make clean
echo "Making"
make triangle_counting_parallel
# make USE_INT=1 page_rank_parallel
sbatch submit.sh
echo
squeue
echo -e "\nOutput\n"
sleep 4
code `ls -t | head -1`
cat `ls -t | head -1`
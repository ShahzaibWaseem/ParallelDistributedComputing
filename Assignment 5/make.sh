echo "Cleaning"
make clean

echo "Making Correctness Scripts"
make one_lock_queue_correctness
make two_lock_queue_correctness
make non_blocking_queue_correctness

echo "Making Throughput Scripts"
make one_lock_queue_throughput
make two_lock_queue_throughput
make non_blocking_queue_throughput

for i in {1..10}
do
    sbatch submit.sh
    if (( $i % 2 == 0 ))
    then
        echo
        squeue
        sleep 37
    fi
done

# code `ls -t | head -1`
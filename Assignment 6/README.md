Assignment 6 \[40 points\]
--------------------------

In this assignment, we will create distributed solutions (i.e., with multiple processes) for Triangle Counting and PageRank using Message Passing Interface (MPI). MPI is different from the shared memory model we have used so far, and requires using APIs to communicate between processes.

Before starting this assignment, you should have completed the [Slurm Tutorial](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/slurm_tutorial/release/index.html), which walks you through how to use our servers for your code development. Additionally, you should also have completed the [MPI tutorial](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/mpi_tutorial/release/index.html), which gives an overview of MPI and how to correctly run MPI programs using slurm.

#### General Instructions:

1.  You are given the serial implementations [here](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment6/assignment6.tar.gz).
2.  MPI permits various communication strategies to pass data between processes. This assignment uses the point-to-point communication strategy which we denote using the commandline option `--strategy 1`. In the next assignment, we will extend the solution with MPI's collective communication strategies.
3.  For simplicity, we only use one thread per process in this assignment. Make sure you use [MPI\_Finalize](https://www.open-mpi.org/doc/current/man3/MPI_Finalize.3.php) before exiting the `main()` function.
4.  MPI uses the distributed model where each process is completely independent and has its own separate memory space. So, we have temporarily increased the memory limit to `10GB`. Remember to set the `--mem` option appropriately in your script.
5.  **While testing your solutions, make sure that `--cpus-per-task` is set to 1 in your slurm job script, and the `--ntasks` and `--nodes` is set based on number of MPI processes and nodes you want.**
    
        #!/bin/bash
        #
        #SBATCH --cpus-per-task=1
        #SBATCH --nodes=1
        #SBATCH --ntasks=4
        #SBATCH --mem=10G
        
        srun ./triangle_counting_parallel
        
    
6.  You will be asked to print the time spent by different threads on specific code regions. The time spent by any code region can be computed as follows:
    
        timer t1;
        t1.start();
        /* ---- Code region whose time is to be measured --- */
        double time_taken = t1.stop();
        
    
    If you need to time a sub-section inside a loop, you can do that as follows:
    
        double time_taken = 0.0;
        timer t1;
        while(True){
            /* ---- Code region whose time should not be measured --- */
        
            t1.start();
            /* ---- Code region whose time is to be measured --- */
            time_taken += t1.stop();
        
            /* ---- Code region whose time should not be measured --- */
        }
        std::cout << "Time spent on required code region : " << time_taken << "\n";
        
    
7.  We have provided test scripts for you to quickly test your solutions during your development process. You can test your code using the test scripts available at `/scratch/assignment6/test_scripts/`. Note that these test scripts only validate the output formats, and a different evaluation script will be used for grading the assignments. **Important: You should always use slurm when performing these tests.** The test scripts under `/scratch/assignment6/test_scripts/` folder test for up to 4 processes on the same node; make sure `--ntasks=4` and `--nodes=1` is set in your slurm job.
    
        $ ls /scratch/assignment6/test_scripts/*tester.pyc
        triangle_counting_tester.pyc page_rank_tester.pyc
        
    
    Here's an example job script to validate your code:
    
        #!/bin/bash
        #
        #SBATCH --cpus-per-task=1
        #SBATCH --nodes=1
        #SBATCH --ntasks=4
        #SBATCH --mem=10G
        
        python /scratch/assignment6/test_scripts/triangle_counting_tester.pyc --execPath=$HOME/my_assignment6_solutions/triangle_counting_parallel
        
    
    **Note that the python script is not invoked with `srun`**. It internally takes care of launching the MPI processes using `srun`.
8.  Sample input graphs are available at `/scratch/input_graphs/`.
    
        $ ls /scratch/input_graphs/*.cs*
        lj.csc  lj.csr  roadNet-CA.csc  roadNet-CA.csr  rmat.csc  rmat.csr
        
    

If you are interested in checking out the original graph datasets (this is not required to solve the assignment), you can find them [here](https://snap.stanford.edu/data/index.html).

9.  If you'd like to test your solution with more graph datasets, you can create your own simple graphs as follows:
    *   Create a file called testGraph.txt with the list of edges (one edge on each line) in the graph. For example,
        
            1 2
            2 3
            
        
    *   Run
        
            $ /scratch/input_graphs/SNAPtoBinary testGraph.txt testGraphConverted
            
        
    *   This will create `testGraphConverted.csr` and `testGraphConverted.csc` files which are [CSR and CSC](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_(CSR,_CRS_or_Yale_format)) representations of the graph.
    *   To use the graphs in your solutions, use the command line argument `--inputFile "testGraphConverted"`.
10.  Since each MPI process is independent, use these rules to print your outputs:
    *   Use `printf()` to avoid garbled logs. You can also concatenate the information as a string and use `std::cout` to print a single line of output. To add a new line, use `"\n"` as part of the concatenated string instead of `std::endl`.
    *   You can check the rank of the process before printing as shown below:
        
            if (world_rank == 0)
                printf("Time taken (in seconds): %g\n", time_taken);
            
        
    *   The root process should print most of the output logs.
    *   Non-root processes should only print the process statistics in a single line.

1\. Triangle Counting using MPI - Point-to-Point \[15 points\]
--------------------------------------------------------------

Similar to [Assignment 1](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment1/index.html), we will develop a parallel solution for Triangle Counting using MPI. Here, the work is distributed among `P` processes. For simplicity, every process will read the entire input graph and create its own graph data structure. We will use **edge decomposition** strategy to distribute the work among the processes (as done in [Assignment 3](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment3/index.html)). Use the following pseudocode for determining the subset of vertices for each process:

    // Edge decomposition strategy for a graph with n vertices and m edges for P processes
    start_vertex=0; end_vertex=0;
    for(i=0; i<P; i++){
        start_vertex=end_vertex;
        long count = 0;
        while (end_vertex < n)
        {
            // add vertices until we reach m/P edges.
            count += g.vertices_[end_vertex].getOutDegree();
            end_vertex += 1;
            if (count >= m/P)
                break;
        }
        if(i == global_rank)
            break;
    }
    // Each process will work on vertices [start_vertex, end_vertex).
    

Each process will compute the number of triangles for the vertices allocated to it. Process 0 (henceforth, referred as the **root process**) aggregates (i.e., sums up) the local counts from other processes (henceforth, referred as **non-root processes**) and computes the final triangle count. The pseudocode for question `1` is given below:

    for each process P in parallel {
        local_count = 0
        for each vertex 'u' allocated to P {
            edges_processed += outDegree(u)  // used in output validation
            for vertex 'v' in outNeighbor(u)
                local_count += count_triangles_for_edge(u, v)
        }
        // --- synchronization phase start ---
        if(P is root process){
            global_count = Sum of local counts of all the processes
        }
        else{
            // depending on the strategy,
            // use appropriate API to send the local_count to the root process
        }
        // --- synchronization phase end -----
    
        if(P is root process){
            global_count = global_count / 3
            // print process statistics and other results
        }
        else{
            // print process statistics
        }
    }
    

You should use point-to-point communication, i.e., [MPI\_Send()](https://www.open-mpi.org/doc/current/man3/MPI_Send.3.php) and [MPI\_Recv()](https://www.open-mpi.org/doc/current/man3/MPI_Recv.3.php), and do the following:

*   Non-root processes will send their `local_count` to the root process.
*   Root process receives the information from other processes and aggregates them to get the final triangle count.

Implement this as `--strategy 1` in your `triangle_counting_parallel` executable.

Note that the MPI function calls we use in this assignment are synchronous calls. So make sure that the [MPI\_Send()](https://www.open-mpi.org/doc/current/man3/MPI_Send.3.php) and [MPI\_Recv()](https://www.open-mpi.org/doc/current/man3/MPI_Recv.3.php) are called in the correct order in every process.

**Output Format for Question 1:**

1.  Your solution should be named `triangle_counting_parallel.cpp` and your Makefile should produce `triangle_counting_parallel` binary. Command line parameters to be supported:
    *   `--inputFile`: The input graph file path as mentioned in the general instructions (datatype: string).
    *   `--strategy`: The strategy to be used (datatype: integer). The valid values for `--strategy` are:
        *   `1` for Point-to-Point
2.  Your parallel solution must output the following information:
    *   World size (i.e., number of processes) (only root process).
    *   Communication strategy used (only root process).
    *   For each process,
        *   Process rank (rank is between \[0, `P`)).
        *   Number of edges processed.
        *   Number of triangles counted (`local_count` in the pseudocode),
        *   Time taken for communication (in seconds). This should include the time taken to sum up the values (if any). Refer the pseudocode.
    *   Total number of triangles in the graph (only root process).
    *   Total number of unique triangles in the graph (only root process).
    *   The total time taken for the entire execution. This should include the communication time and decomposition time (only root process).

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as shown below. You can run the python script only with slurm. Remember to invoke the script without `srun`.

    $ python /scratch/assignment6/test_scripts/triangle_counting_tester.pyc --execPath=<absolute path of triangle_counting_parallel>
    

**Sample output**

    World size : 4
    Communication strategy : 1
    1, 1383303, 125397, 0.000026
    rank, edges, triangle_count, communication_time
    0, 1383303, 161925, 0.007404
    Number of triangles : 597206
    Number of unique triangles : 199068
    3, 1383305, 154830, 0.000005
    Time taken (in seconds) : 0.08227
    2, 1383303, 155054, 0.000005
    

2\. PageRank using MPI - Point-to-Point \[25 points\]
-----------------------------------------------------

Similar to Triangle Counting, we will implement PageRank with MPI. Here, the work is distributed among `P` processes. For simplicity, every process will read the entire input graph and create its own graph data structure. We will use **edge decomposition** strategy to distribute the work among the processes (as done in [Assignment 3](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment3/index.html)). Refer to the pseudocode in the `Triangle Counting using MPI` section for the decomposition strategy.

The PageRank pseudocode is given below:

    for each process P in parallel {
        communication_time = 0.0;
        for(i=0; i<max_iterations; i++) {
            for each vertex 'u' allocated to P {  // Loop 1
                edges_processed += outDegree(u)  // used in output validation
                for vertex 'v' in outNeighbor(u){
                    next_page_rank[v] += (current_page_rank[u]/outdegree[u])
                }
            }
    
            // --- synchronization phase 1 start ---
            timer1.start();
            for each vertex 'u' allocated to P, aggregate (i.e., sum up) the value of next_page_rank[u] from all processes
            communication_time += timer1.stop();
            // --- synchronization phase 1 end -----
    
            for each vertex 'v' allocated to P {  // Loop 2
                new_value = PAGERANK(next_page_rank[v])
                current_page_rank[v] = new_value
            }
            Reset next_page_rank[v] to 0 for all vertices
        }
    }
    local_sum = 0
    for each vertex 'v' allocated to P {  // Loop 3
        local_sum += current_page_rank[v]
    }
    // --- synchronization phase 2 start ---
    if(P is root process){
        global_sum = Aggregated value of local_sum from all the processes 
        // print process statistics and other results
    }
    else{
        // print process statistics.
    }
    // --- synchronization phase 2 end ---
    
    

**Key things to note:**

1.  For `Loop 1` and `Loop 2`, each process only works on the vertices (and corresponding out edges) allocated to it.
2.  You need to reset `next_page_rank` for all vertices at the end of every iteration.
3.  The `synchronization phase 1` is performed by all processes. For this assignment we will use the point-to-point strategy (described below).
4.  You should not perform synchronization individually for each vertex. Since each process works on a contiguous subset of vertices, you should synchronize the entire subset of vertices.
5.  The `global_sum` computed in `synchronization phase 2` is used for output verification. Use the same point-to-point communication method to transfer the `local_sum` of each process to the root process.
6.  Only the time spent on `synchronization phase 1` is used for calculating communication time.

You should use point-to-point communication, i.e., [MPI\_Send()](https://www.open-mpi.org/doc/current/man3/MPI_Send.3.php) and [MPI\_Recv()](https://www.open-mpi.org/doc/current/man3/MPI_Recv.3.php) to communicate between processes. In this strategy,

*   All non-root processes send the `next_page_rank` value for **all vertices** to the root process.
*   The root process receives the `next_page_rank` values for all vertices and uses it to sum up the `next_page_rank` value for every vertex in the graph.
*   The root process sends the aggregated `next_page_rank` value of each vertex to its appropriate process. For example, if vertex `v` is allocated to process `P1`, `next_page_rank[v]` is sent only to `P1`.

Implement this as `--strategy 1` in your `page_rank_parallel` executable.

Note that the MPI function calls we use in this assignment are synchronous calls. So make sure that the [MPI\_Send()](https://www.open-mpi.org/doc/current/man3/MPI_Send.3.php) and [MPI\_Recv()](https://www.open-mpi.org/doc/current/man3/MPI_Recv.3.php) are called in the correct order in every process.

**Output Format for Question 2:**

1.  Your solution should be named `page_rank_parallel.cpp` and your Makefile should produce `page_rank_parallel` binary. Command line parameters to be supported:
    *   `--inputFile`: The input graph file path as mentioned in the general instructions (datatype: string).
    *   `--nIterations`: The number of iterations to run PageRank (datatype: integer).
    *   `--strategy`: The strategy to be used (datatype: integer). The valid values for --strategy are:
        *   `1` for Point-to-Point communication.
2.  Your parallel solution must output the following information:
    *   World size (i.e., number of processes) (only root process).
    *   Communication strategy used. (only root process).
    *   For each process,
        *   Process rank (your rank is between \[0, `P`)).
        *   Number of edges processed (across all iterations) - This is the number of edges processed only in the first `for` loop across all iterations. Refer the pseudocode of PageRank.
        *   Cumulative time taken for communication (in seconds). This should include the time taken to sum up the values (if any). Refer the pseudocode.
    *   The sum of pageranks of all vertices (only root process).
    *   The total time taken for the entire execution. This should include the communication time (only root process).
3.  We will be using the integer version of PageRank for evaluating correctness. To run the integer version of PageRank, use the flag `USE_INT=1` during `make` as follows:
    
        $ make USE_INT=1 page_rank_parallel
        
    

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as shown below. You can run the python script only with slurm. Remember to invoke the script without `srun`.

    $ python /scratch/assignment6/test_scripts/page_rank_tester.pyc --execPath=<absolute path of page_rank_parallel>
    

**Sample output:**

    Using FLOAT
    World size : 4
    Communication strategy : 2
    Iterations : 20
    rank, num_edges, communication_time
    0, 27666060, 0.748060
    1, 27666060, 0.749769
    2, 27666060, 0.733355
    3, 27666100, 0.738441
    Sum of page rank : 1966528.125000
    Time taken (in seconds) : 1.26269
    

### Submission Guidelines

*   Make sure that your solutions folder has the following files and sub-folders. Let's say your solutions folder is called `my_assignment6_solutions`. It should contain:
    *   `core/` -- The folder containing all core files. It is already available in the assignment 6 package. Do not modify it or remove any files.
    *   `Makefile` -- Makefile for the project. This is the same Makefile provided in the serial package. Do not modify it.
    *   `triangle_counting_parallel.cpp`
    *   `page_rank_parallel.cpp`
*   To create the submission file, follow the steps below:
    1.  Enter in your solutions folder, and remove all the object/temporary files.
        
            $ cd my_assignment6_solutions/
            $ make clean
            
        
    2.  Create the tar.gz file.
        
            $ tar cvzf assignment6.tar.gz *
            
        
        which creates a compressed tar ball that contains the contents of the folder.
    3.  Validate the tar ball using the `submission_validator.pyc` script.
        
            $ python /scratch/assignment6/test_scripts/submission_validator.pyc --tarPath=assignment6.tar.gz
            
        
*   Submit via [CourSys](https://courses.cs.sfu.ca/) by the deadline posted there.

* * *

Copyright © 2020 Keval Vora. All rights reserved.
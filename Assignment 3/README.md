Assignment 3 \[55 + 45 Points\]
-------------------------------

In [Assignment 1](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignment1/release/index.html) and [Assignment 2](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignment2/release/index.html), we developed parallel solutions for Triangle Counting and PageRank. In this assignment, we will improve those parallel solutions. Specifically, we will look at different task decomposition and mapping strategies for Triangle Counting and PageRank programs. You will implement and observe the effects of different strategies, and write a report that answers the specific questions listed here [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing) (submission guidelines available at the bottom of this page).

Before starting this assignment, you should have completed the [Slurm Tutorial](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/slurm_tutorial/release/index.html) which walks you through how to use our servers for your code development.

Performance of a program is often sensitive to the way in which data is laid out in memory. [Tutorial 2](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/data_layout/release/index.html) explains how data layout in memory can affect performance. You should ensure your solutions do not suffer from false sharing, and are not limited by poor choice of data layout.

### General Instructions

1.  You are provided with the serial version of all the programs, and the sample outputs [here](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment3/assignment3.tar.gz).
2.  Since we will be dealing with multiple strategies for both, Triangle Counting and PageRank, we will distinguish those strategies using `--strategy` command line argument. The value of `--strategy` will be either `1` or `2` (more details presented with each strategy below).
3.  All parallel programs should have the command-line argument `--nWorkers` to specify the number of threads for the program. Example: `--nWorkers 4`.
4.  **While testing your solutions, make sure that `cpus-per-task` is correctly specified in your slurm config file based on your requirement.**
5.  You will be asked to print the time spent by different threads on specific code regions. The time spent by any code region can be computed as follows:
    
        timer t1;
        t1.start();
        /* ---- Code region whose time is to be measured --- */
        double time_taken = t1.stop();
        
    
6.  If you need to time a sub-section inside a loop, you can do that as follows:
    
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
        
    
7.  We have provided test scripts for you to quickly test your solutions during your development process. You can test your code using the test script available at `/scratch/assignment3/test_scripts/`. Note that these test scripts only validate the output formats, and a different evaluation script will be used for grading the assignments. **Important: You should use slurm when performing these and other tests**. The test scripts under `/scratch/assignment3/test_scripts/` folder test for up to 4 threads; make sure `--cpus-per-task=4` is set in your slurm job.
    
        $ ls /scratch/assignment3/test_scripts/*tester.pyc
        triangle_counting_tester.pyc page_rank_tester.pyc
        
    
8.  The programs operate on graph datasets. Sample input graphs are available at `/scratch/input_graphs/` on the compute nodes (note they are present on the compute nodes only, and hence you can access them via slurm only).
    
        $ ls /scratch/input_graphs/*.cs*
        lj.csc  lj.csr  roadNet-CA.csc  roadNet-CA.csr  rmat.csc  rmat.csr
        
    
    If you are interested in checking out the original graph datasets (this is not required to solve the assignment), you can find them [here](https://snap.stanford.edu/data/index.html).
9.  If you'd like to test your solution with more graph datasets, you can create your own simple graphs as follows:
    *   Create a file called `testGraph.txt` with the list of edges (one edge on each line) in the graph. For example,
        
            1 2
            2 3
            
        
    *   Run
        
            $ /scratch/input_graphs/SNAPtoBinary testGraph.txt testGraphConverted
            
        
    *   This will create `testGraphConverted.csr` and `testGraphConverted.csc` files which are [CSR and CSC](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_(CSR,_CRS_or_Yale_format)) representations of the graph.
    *   To use the graphs in your solutions, use the command line argument `--inputFile "testGraphConverted"`.

### 1\. Triangle Counting - Vertex-based Task Decomposition \[5 Points\]

In [Assignment 1](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignment1/release/index.html), we developed a parallel solution for Triangle Counting where each thread works on (approximately) `n/T` vertices, where `n` is the number of vertices in the graph and `T` is the number of threads. The pseudocode of this solution is shown below.

        Create T threads
        for each thread in parallel {
            Compute the number of triangles for (approximately) n/T vertices allocated to the thread
        }
        triangle_count = Accumulate the triangle counts from all the threads
        triangle_count = triangle_count / 3
    

This is the naive `vertex-based task decomposition` strategy, which will be used with `--strategy 1` (i.e., add `--strategy 1` in your solution). Answer question 1 in [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing), which will require you to run your solution and analyze the results.

### 2\. Triangle Counting - Edge-based Task Decomposition \[25 Points\]

In this strategy, you have to distribute the `m` edges in the graph such that each thread works on approximately `m/T` edges, where `T` is the number of workers. Below is the pseudo-code showing edge-based task decomposition:

        Create T threads
        for each thread in parallel {
            Compute the number of triangles created by approximately m/T edges allocated to the thread
        }
        triangle_count = Accumulate the triangle counts from all the threads
        triangle_count = triangle_count / 3
    

This strategy should be used with command-line parameter `--strategy 2`. Implement the above strategy in your solution and answer question 2 in [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing).

**Output Format for Questions 1-2:**

1.  Your solution should be named `triangle_counting_parallel.cpp` and your Makefile should produce `triangle_counting_parallel` binary. Command line parameters to be supported:
    *   `--nWorkers`: The number of threads. (datatype: integer).
    *   `--inputFile`: The input graph file path as mentioned in the general instructions. (datatype: string).
    *   `--strategy`: The strategy to be used (datatype: integer). The valid values for `--strategy` are:
        *   `1` for vertex-based task decomposition.
        *   `2` for edge-based task decomposition.
2.  Your parallel solution must output the following information:
    *   Total number of threads used.
    *   Task decomposition/mapping strategy used.
    *   For each thread:
        *   Thread id (your threads should be numbered between `[0, T)`)
        *   Number of vertices processed (Print 0 for edge-based task decomposition)
        *   Number of edges processed
        *   Number of triangles counted
        *   Time taken by the thread
    *   Total number of triangles in the graph.
    *   Total number of unique triangles in the graph.
    *   Time taken for the task decomposition/mapping. For example, if your vertex-based decomposition strategy is implemented across two functions `get_start_vertex()` and `get_end_vertex()`, you should time them as follows:
        
            int start_vertex = 0; end_vertex = 0;
            timer t2;
            double partitionTime = 0.0;
            for(int tid=0; tid<number_of_workers; tid++){
                t2.start();
                start_vertex = get_start_vertex(tid);
                end_vertex = get_end_vertex(tid);
                partitionTime += t2.stop();
            }
            std::cout << "Partitioning time (in seconds) : " << partitioning_time << "\n";
            
        
        If your task decomposition/mapping is carried out in the thread function, then get the time taken for task decomposition/mapping by thread 0 and print it in the main thread.
        
            // Thread function
            void threadFunction(int tid, double* partitionTime){
                int start_vertex = 0; end_vertex = 0;
                timer t2;
                t2.start();
                start_vertex = get_start_vertex(tid);
                end_vertex = get_end_vertex(tid);
                double t_partitionTime = t2.stop();
                if(tid == 0){
                    *partitionTime = t_partitionTime;
                }
                // --- Other code ---
            }
            
            int main(){
                double partitionTime = 0.0;
                // --- Other code ---
                for (uint i = 0; i < num_of_workers; i++){
                    threads[i] = std::thread(threadFunction, tid, &partitionTime);
                }
                // --- Other code ---
                std::cout << "Partitioning time (in seconds) : " << partitioning_time << "\n";
            }
            
        
    *   The total time taken for the entire execution. This should include the time taken by your task decomposition/mapping strategy irrespective of whether it is performed by the main thread or by worker threads.
3.  The sample console output can be found in `sample_outputs/triangle_counting.output`.

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as follows (remember to run this via slurm):

    $ python /scratch/assignment3/test_scripts/triangle_counting_tester.pyc --execPath=<absolute path of triangle_counting_parallel> 
    

### 3\. PageRank - Vertex-based Task Decomposition \[5 Points\]

In [Assignment 2](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignment2/release/index.html), we developed a parallel solution for PageRank using `std::atomic` where each thread works on (approximately) `n/T` vertices, where `n` is the number of vertices in the graph and `T` is the number of threads. The pseudocode of this solution is shown below.

        Create T threads
        Distribute vertices to each thread such that each thread works on approximately n/T vertices
        for each thread in parallel {
            for(i=0; i<max_iterations; i++) {
                for each vertex 'u' allocated to the thread {
                    edges_processed += outDegree(u) // used in output validation
                    for vertex 'v' in outNeighbor(u)
                        next_page_rank[v] += (current_page_rank[u]/outdegree[u]) 
                }
                barrier1 // barrier1_time -> cumulative time spent on this line
                
                for each vertex 'v' allocated to the thread {
                    vertices_processed += 1 // used in output validation
                    compute the new_pagerank using the accumulated values in next_page_rank[v].
                    current_page_rank[v] = new_pagerank
                    Reset next_page_rank[v] to 0
                }
                barrier2 // barrier2_time -> cumulative time spent on this line
            }
        }
    

This is the naive `vertex-based task decomposition` strategy, which will be used with `--strategy 1` (i.e., add `--strategy 1` in your solution). Answer questions 3 and 4 in [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing), which will require you to run your solution and analyze the results.

### 4\. PageRank - Edge-based Task Decomposition \[20 Points\]

In this strategy, you have to distribute the vertices to each thread such that each thread is allocated approximately `m/T` edges, where `m` is the number of edges in the graph and `T` is the number of workers. Below is the pseudo-code showing edge-based task decomposition:

        Create T threads
        Distribute vertices to each thread such that each thread works on approximately m/T edges
        for each thread in parallel {
            for(i=0; i<max_iterations; i++) {
                for each vertex 'u' allocated to the thread {
                    edges_processed += outDegree(u) // used in output validation
                    for vertex 'v' in outNeighbor(u)
                        next_page_rank[v] += (current_page_rank[u]/outdegree[u]) 
                }
                barrier1
    
                for each vertex 'v' allocated to the thread {
                    vertices_processed += 1 // used in output validation
                    compute the new_pagerank using the accumulated values in next_page_rank[v].
                    current_page_rank[v] = new_pagerank
                    Reset next_page_rank[v] to 0
                }
                barrier2
            }
        }
    

This strategy should be used with command-line parameter `--strategy 2`. Implement the above strategy in your solution and answer question 5 in [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing).

**Output Format for Questions 3-4:**

1.  Your solution should be named `page_rank_parallel.cpp` and your Makefile should produce `page_rank_parallel` binary. Command line parameters to be supported:
    *   `--nWorkers`: The number of threads. (datatype: integer).
    *   `--inputFile`: The input graph file path as mentioned in the general instructions. (datatype: string).
    *   `--strategy`: The strategy to be used (datatype: integer). The valid values for `--strategy` are:
        *   `1` for vertex-based task decomposition.
        *   `2` for edge-based task decomposition.
2.  Your parallel solution must output the following information:
    *   Total number of threads used.
    *   Task decomposition/mapping strategy used.
    *   For each thread:
        *   Thread id (your threads should be numbered between `[0, T)`)
        *   Number of vertices processed (across all iterations) - This is the number of vertices processed only in the second `for` loop across all iterations. Refer the pseudocode of pagerank.
        *   Number of edges processed (across all iterations) - This is the number of edges processed only in the first `for` loop across all iterations. Refer the pseudocode of pagerank.
        *   Cumulative time spent waiting at `barrier1` (in seconds)
        *   Cumulative time spent waiting at `barrier2` (in seconds)
        *   Time taken by the thread (in seconds). This should include the time taken by your task decomposition/mapping strategy if it is carried out in the thread.
    *   The sum of pageranks of all vertices.
    *   Time taken for the task decomposition/mapping. For example, if your vertex-based decomposition strategy is implemented across two functions `get_start_vertex()` and `get_end_vertex()`, you should time them as follows:
        
            int start_vertex = 0; end_vertex = 0;
            timer t2;
            double partitionTime = 0.0;
            for(int tid=0; tid<number_of_workers; tid++){
                t2.start();
                start_vertex = get_start_vertex(tid);
                end_vertex = get_end_vertex(tid);
                partitionTime += t2.stop();
            }
            std::cout << "Partitioning time (in seconds) : " << partitioning_time << "\n";
            
        
        If your task decomposition/mapping is carried out in the thread function, then get the time taken for task decomposition/mapping by thread 0 and print it in the main thread.
        
            // Thread function
            void threadFunction(int tid, double* partitionTime){
                int start_vertex = 0; end_vertex = 0;
                timer t2;
                t2.start();
                start_vertex = get_start_vertex(tid);
                end_vertex = get_end_vertex(tid);
                double t_partitionTime = t2.stop();
                if(tid == 0){
                    *partitionTime = t_partitionTime;
                }
                // --- Other code ---
            }
            
            int main(){
                double partitionTime = 0.0;
                // --- Other code ---
                for (uint i = 0; i < num_of_workers; i++){
                    threads[i] = std::thread(threadFunction, tid, &partitionTime);
                }
                // --- Other code ---
                std::cout << "Partitioning time (in seconds) : " << partitioning_time << "\n";
            }
            
        
    *   The total time taken for the entire execution. This should include the time taken by your task decomposition/mapping strategy irrespective of whether it is performed by the main thread or by worker threads.
3.  The sample output can be found in `sample_outputs/page_rank.output`.

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as follows (remember to run this via slurm):

    $ python /scratch/assignment3/test_scripts/page_rank_tester.pyc --execPath=<absolute path of page_rank_parallel>
    

### Submission Guidelines

*   Make sure that your solutions folder has the following files and sub-folders. Let's say your solutions folder is called `my_assignment3_solutions`. It should contain:
    *   `core/` -- The folder containing all core files. It is already available in the assignment package. Do not modify it or remove any files.
    *   `Makefile` -- Makefile for the project.
    *   `triangle_counting_parallel.cpp`
    *   `page_rank_parallel.cpp`
*   To create the submission file, follow the steps below:
    1.  Enter in your solutions folder, and remove all the object/temporary files.
        
            $ cd my_assignment3_solutions/
            $ make clean
            
        
    2.  Create the tar.gz file.
        
            $ tar cvzf assignment3.tar.gz *
            
        
        which creates a compressed tar ball that contains the contents of the folder.
    3.  Validate the tar ball using the `submission_validator.pyc` script.
        
            $ python /scratch/assignment3/test_scripts/submission_validator.pyc --tarPath=assignment3.tar.gz
            
        
*   For assignment report,
    *   Create a copy of [Assignment 3 - Report](https://docs.google.com/document/d/1B3deSx4ifIZkbv5O6hknWHH5mOi7aRmOKVj2QgjM8aQ/edit?usp=sharing).
    *   Fill in your answers.
    *   Select `File -> Download -> PDF Document`. Save the downloaded file as `report.pdf`.
*   Submit via [CourSys](https://courses.cs.sfu.ca/) by the deadline posted there.

* * *

Copyright © 2020 Keval Vora. All rights reserved.
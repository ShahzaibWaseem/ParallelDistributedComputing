Assignment 4 \[55 + 45 Points\]
-------------------------------

In [Assignment 3](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment3/index.html), we developed parallel solutions for Triangle Counting and PageRank with Vertex/Edge-based Decomposition. In this assignment, we will develop parallel solutions with dynamic task decomposition and mapping strategies. You will implement and observe the effects of the strategies, and write a report that answers the specific questions listed here [Assignment 4 - Report](https://docs.google.com/document/d/1Cgj0hJktOL3ta4kmDgQ0EjT5YSrb2ufReo2qWSOur1s/edit?usp=sharing) (submission guidelines available at the bottom of this page).

Before starting this assignment, you should have completed the [Slurm Tutorial](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/slurm_tutorial/release/index.html) which walks you through how to use our servers for your code development.

### General Instructions

1.  You are provided with the serial version of all the programs, and the sample outputs [here](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment4/assignment4.tar.gz).
2.  All parallel programs should have the command-line argument `--nWorkers` to specify the number of threads for the program. Example: `--nWorkers 4`.
3.  **While testing your solutions, make sure that `cpus-per-task` is correctly specified in your slurm config file based on your requirement.**
4.  You will be asked to print the time spent by different threads on specific code regions. The time spent by any code region can be computed as follows:
    
        timer t1;
        t1.start();
        /* ---- Code region whose time is to be measured --- */
        double time_taken = t1.stop();
        
    
5.  If you need to time a sub-section inside a loop, you can do that as follows:
    
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
        
    
6.  We have provided test scripts for you to quickly test your solutions during your development process. You can test your code using the test script available at `/scratch/assignment4/test_scripts/`. Note that these test scripts only validate the output formats, and a different evaluation script will be used for grading the assignments. **Important: You should use slurm when performing these and other tests**. The test scripts under `/scratch/assignment4/test_scripts/` folder test for up to 4 threads; make sure `--cpus-per-task=4` is set in your slurm job.
    
        $ ls /scratch/assignment4/test_scripts/*tester.pyc
        triangle_counting_tester.pyc page_rank_tester.pyc
        
    
7.  Sample input graphs are available at `/scratch/input_graphs/`.
    
        $ ls /scratch/input_graphs/*.cs*
        lj.csc  lj.csr  roadNet-CA.csc  roadNet-CA.csr  rmat.csc  rmat.csr
        
    

If you are interested in checking out the original graph datasets (this is not required to solve the assignment), you can find them [here](https://snap.stanford.edu/data/index.html).

8.  If you'd like to test your solution with more graph datasets, you can create your own simple graphs as follows:
    *   Create a file called `testGraph.txt` with the list of edges (one edge on each line) in the graph. For example,
        
            1 2
            2 3
            
        
    *   Run
        
            $ /scratch/assignment4/input_graphs/SNAPtoBinary testGraph.txt testGraphConverted
            
        
    *   This will create `testGraphConverted.csr` and `testGraphConverted.csc` files which are [CSR and CSC](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_(CSR,_CRS_or_Yale_format)) representations of the graph.
    *   To use the graphs in your solutions, use the command line argument `--inputFile "testGraphConverted"`.

### 1\. Triangle Counting - Dynamic Task Mapping \[25 Points\]

Instead of allocating approximately equal number of vertices (or edges) to threads, we can dynamically allocate work to each thread whenever it is free. In this strategy, each thread dynamically gets the next vertex to be computed until all the vertices are processed. Below is the pseudo-code showing dynamic task mapping (with vertex-based decomposition):

        Create T threads
        for each thread in parallel {
            while(true){
                v = getNextVertexToBeProcessed()
                if(v == -1) break;
                Compute the number of triangles created by the vertex v & its outNeighbors
            }
        }
        triangle_count = Accumulate the triangle counts from all the threads
        triangle_count = triangle_count / 3
    

Implement the above strategy in your solution and answer question 1 in [Assignment 4 - Report](https://docs.google.com/document/d/1Cgj0hJktOL3ta4kmDgQ0EjT5YSrb2ufReo2qWSOur1s/edit?usp=sharing).

**Output Format for Question 1:**

1.  Your solution should be named `triangle_counting_parallel.cpp` and your Makefile should produce `triangle_counting_parallel` binary. Command line parameters to be supported:
    *   `--nWorkers`: The number of threads. (datatype: integer).
    *   `--inputFile`: The input graph file path as mentioned in the general instructions. (datatype: string).
2.  Your parallel solution must output the following information:
    *   Total number of threads used.
    *   For each thread:
        *   Thread id (your threads should be numbered between `[0, T)`)
        *   Number of vertices processed
        *   Number of edges processed
        *   Number of triangles counted
        *   Time taken by the thread
    *   Total number of triangles in the graph.
    *   Total number of unique triangles in the graph.
    *   The total time taken for the entire execution.
3.  The sample console output can be found in `sample_outputs/triangle_counting.output`.

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as follows:

    $ python /scratch/assignment4/test_scripts/triangle_counting_tester.pyc --execPath=<absolute path of triangle_counting_parallel> 
    

### 2\. PageRank - Dynamic Task Mapping \[15 Points\]

Instead of allocating approximately equal number of vertices (or edges) to threads, we can dynamically allocate work to each thread whenever it is free. In this strategy, each thread dynamically gets the next vertex to be computed until all the vertices are processed. Below is the pseudo-code showing dynamic task mapping (with vertex-based decomposition):

        Create T threads
        for each thread in parallel {
            for(i=0; i<max_iterations; i++) {
                while(true){
                    u = getNextVertexToBeProcessed();
                    if(u == -1) break;
                    edges_processed += outDegree(u) // used in output validation
                    for vertex v in outNeighbor(u) 
                        next_page_rank[v] += (current_page_rank[u]/outdegree[u]) 
                }
                barrier1
    
                while(true){
                    v = getNextVertexToBeProcessed();
                    if(v == -1) break;
                    vertices_processed += 1 // used in output validation
                    compute the new_pagerank using the accumulated values in next_page_rank[v].
                    current_page_rank[v] = new_pagerank
                    Reset next_page_rank[v] to 0
                }
                barrier2
            }
        }
    

Implement the above strategy in your solution and answer questions 2 and 3 in [Assignment 4 - Report](https://docs.google.com/document/d/1Cgj0hJktOL3ta4kmDgQ0EjT5YSrb2ufReo2qWSOur1s/edit?usp=sharing).

### 3\. PageRank - Granularity for Dynamic Task Mapping \[15 Points\]

To reduce the time spent by each thread on the `getNextVertexToBeProcessed()`, we will vary the task granularity so that each thread receives multiple vertices to be processed each time it calls `getNextVertexToBeProcessed()`.

Update the dynamic load distribution logic as follows:

*   Each thread processes `k` vertices and then calls `getNextVertexToBeProcessed()`. Here, `k` determines the granularity of the work done by each thread before requesting new work. For example,
    *   If `k = 1`, the thread calls `getNextVertexToBeProcessed()` after processing each vertex.
    *   If `k = 1000`, the thread calls `getNextVertexToBeProcessed()` after processing 1000 vertices.
*   The `getNextVertexToBeProcessed()` function should return `0`, `k`, `2k`, ... depending on the granularity `k`.
*   `k` should be provided at run time using command-line parameter. Eg: `--granularity 100`

Below is the pseudo-code showing the logic of our parallel solution:

        k = 1000 // granularity
        Create T threads
        for each thread in parallel {
            for(i=0; i<max_iterations; i++) {
                while(true){
                    u = getNextVertexToBeProcessed() 
                    if(u == -1) break;
                    for (j = 0; j < k; j++) {
                        edges_processed += outDegree(u) // used in output validation
                        for vertex v in outNeighbor(u) 
                            next_page_rank[v] += (current_page_rank[u]/outdegree[u] 
                        u++
                        if(u >= n) break; // n is the total number of vertices in the graph
                    }
                }
                barrier1
                while(true){
                    v = getNextVertexToBeProcessed()
                    if(v == -1) break;
                    for (j = 0; j < k; j++) {
                        vertices_processed += 1 // used in output validation
                        compute the new_pagerank using the accumulated values in next_page_rank[v].
                        current_page_rank[v] = new_pagerank
                        Reset next_page_rank[v] to 0
                        v++
                        if(v >= n) break; // n is the total number of vertices in the graph
                    }
                }
                barrier2
            }
        }
    

This strategy should be used with command-line parameter `--granularity` to specify the granularity. Implement the above strategy in your solution and answer questions 4 and 5 in [Assignment 4 - Report](https://docs.google.com/document/d/1Cgj0hJktOL3ta4kmDgQ0EjT5YSrb2ufReo2qWSOur1s/edit?usp=sharing).

**Output Format for Questions 2-3:**

1.  Your solution should be named `page_rank_parallel.cpp` and your Makefile should produce `page_rank_parallel` binary. Command line parameters to be supported:
    *   `--nWorkers`: The number of threads. (datatype: integer).
    *   `--inputFile`: The input graph file path as mentioned in the general instructions. (datatype: string).
    *   `--nIterations`: The number of iterations for PageRank.Use `20` as the default. (datatype: integer).
    *   `--granularity`: The granularity to be used for the dynamic task mapping. Default granularity should be 1. (datatype: integer).
2.  Your parallel solution must output the following information:
    *   Total number of threads used.
    *   Granularity used.
    *   Number of iterations for PageRank.
    *   For each thread:
        *   Thread id (your threads should be numbered between `[0, T)`)
        *   Number of vertices processed (across all iterations) - This is the number of vertices processed only in the second `for` loop across all iterations. Refer the pseudocode of pagerank.
        *   Number of edges processed (across all iterations) - This is the number of edges processed only in the first `for` loop across all iterations. Refer the pseudocode of pagerank.
        *   Cumulative time spent waiting at `barrier1` (in seconds)
        *   Cumulative time spent waiting at `barrier2` (in seconds)
        *   Cumulative time spent waiting at `getNextVertexToBeProcessed()` (in seconds).
        *   Time taken by the thread (in seconds).
    *   The sum of pageranks of all vertices.
    *   The total time taken for the entire execution.
3.  The sample output can be found in `sample_outputs/page_rank.output`.

Please note that the output format should strictly match the expected format (including "spaces" and "commas"). You can test your code using the test script as follows:

    $ python /scratch/assignment4/test_scripts/page_rank_tester.pyc --execPath=<absolute path of page_rank_parallel>
    

### Submission Guidelines

*   Make sure that your solutions folder has the following files and sub-folders. Let's say your solutions folder is called `my_assignment4_solutions`. It should contain:
    *   `core/` -- The folder containing all core files. It is already available in the assignment 1 package. Do not modify it or remove any files.
    *   `Makefile` -- Makefile for the project. This file should not be changed.
    *   `triangle_counting_parallel.cpp`
    *   `page_rank_parallel.cpp`
*   To create the submission file, follow the steps below:
    1.  Enter in your solutions folder, and remove all the object/temporary files.
        
            $ cd my_assignment4_solutions/
            $ make clean
            
        
    2.  Create the tar.gz file.
        
            $ tar cvzf assignment4.tar.gz *
            
        
        which creates a compressed tar ball that contains the contents of the folder.
    3.  Validate the tar ball using the `submission_validator.pyc` script.
        
            $ python /scratch/assignment4/test_scripts/submission_validator.pyc --tarPath=assignment4.tar.gz
            
        
*   For assignment report,
    *   Create a copy of [Assignment 4 - Report](https://docs.google.com/document/d/1Cgj0hJktOL3ta4kmDgQ0EjT5YSrb2ufReo2qWSOur1s/edit?usp=sharing).
    *   Fill in your answers.
    *   Select `File -> Download -> PDF Document`. Save the downloaded file as `report.pdf`.
*   Submit via [CourSys](https://courses.cs.sfu.ca/) by the deadline posted there.

* * *

Copyright © 2020 Keval Vora. All rights reserved.
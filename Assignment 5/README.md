 

Assignment 5 \[20 + 20 Points\]
-------------------------------

In this assignment, we will implement and evaluate concurrent queues. We will look at two lock-based queues and one lockless queue. Your task is to implement and observe the performance of these queues, and write a report that answers the specific questions listed here [Assignment 5 - Report](https://docs.google.com/document/d/1giEGNdJDU8YdgMvje6HSdOGUVnlFun50MaX6XLSLgbY/edit?usp=sharing) (submission guidelines available at the bottom of this page).

Before starting this assignment, you should have completed the [Slurm Tutorial](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/slurm_tutorial/release/index.html) which walks you through how to use our servers for your code development.

### General Instructions

1.  Read this paper: [Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms](https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf). We will be implementing and evaluating the queues proposed in this paper.
2.  Download the assignment code (queue interfaces, driver program, etc.) [here](https://www.cs.sfu.ca/~keval/teaching/cmpt770/fall21/assignments/assignment5/assignment5.tar.gz).
3.  The queue interfaces are provided to you in following files:
    *   `queues/one_lock_queue.h`
    *   `queues/two_lock_queue.h`
    *   `queues/non_blocking_queue.h`
4.  You are also given two driver programs to evaluate the correctness and performance of your queue implementations. You should not modify these files for your submission.

*   `driver_correctness.cpp` - In this driver program, producers repeatedly enqueue elements from an input file into the queue and consumers repeatedly dequeue elements from the queue. The driver program then validates whether the enqueued and dequeued elements are correct. It supports the following command-line parameters:
    
    *   `--n_producers`: Number of producer threads that will enqueue to the queue.
    *   `--n_consumers`: Number of consumer threads that will dequeue from the queue.
    *   `--input_file`: Absolute path of the input file that contains the elements to be inserted.
    *   `--init_allocator`: Initial number of elements to be pre-allocated. We will rely on pre-allocation to eliminate the effects of allocation/deallocation on throughput numbers (more on this in next point).
    
    For example, to test correctness of `one_lock_queue` with 2 producers and 2 consumers, run:
    
        $ make one_lock_queue_correctness
        $ ./one_lock_queue_correctness --n_producers 2 --n_consumers 2 --input_file /scratch/assignment5/inputs/rand_10M
        
    
*   `driver_throughput.cpp` - In this driver program, producers repeatedly enqueue elements in the queue and consumers repeatedly dequeue elements from the queue for a specified period of time. The number of operations performed by the workers is used to measure throughput of the queue. It supports the following command-line parameters:
    
    *   `--seconds`: Number of seconds the producer and consumer should run.
    *   `--n_producers`: Number of producer threads that will enqueue to the queue.
    *   `--n_consumers`: Number of consumer threads that will dequeue from the queue.
    *   `--init_allocator`: Initial number of elements to be pre-allocated. We will rely on pre-allocation to eliminate the effects of allocation/deallocation on throughput numbers (more on this in next point).
    
    For example, to measure the throughput of `non_blocking_queue` with 2 producers and 2 consumers for 5 seconds, run:
    
        $ make non_blocking_queue_throughput
        $ ./non_blocking_queue_throughput --n_producers 2 --n_consumers 2 --seconds 5 --init_allocator 100000000
        
    

5.  We will use a custom allocator for allocating and deallocating elements in the queue. The custom allocator is thread-safe, and hence, you don't need to worry about synchronizing allocation/deallocation calls. The custom allocator can be used as follows:
    
            CustomAllocator my_allocator;
            // You can initialize the allocator with the number of elements to be pre-allocated and the size of each element
            my_allocator.initialize(1, sizeof(Node<int>));  // Here, 1 element of sizeof(Node<int>) is pre-allocated
            
            // Creating a node using the allocator
            // NOTE: Must typecast explicitly to the desired type as shown below
            Node<int> *new_node = (Node<int>*)my_allocator.newNode();
            // Always set the member variables of the element as the custom allocator does not invoke any constructor.
            new_node.value = 5;
            new_node.next = nullptr;
            
            // Creating another node will cause failure as my_allocator is initialized with only 1 node
            Node<int> *new_node2 = (Node<int>*)my_allocator.newNode();     // This will fail.
            
            // Freeing the allocated node
            my_allocator.freeNode(new_node);
            
            // Cleanup the custom allocator
            my_allocator.cleanup();
        
    
6.  The queue interfaces provided to you have the custom allocator object along with the appropriate `initialize()` and `cleanup()` calls. Ensure that you use the correct template type for the custom allocator object depending on the node type used by your queue implementation.
7.  Sample inputs are available at `/scratch/assignment5/inputs/`. These inputs contain a list of items to be inserted in the queue.
8.  You can generate your own dataset by running:
    
        $ /scratch/assignment5/inputs/generate_test_input --nValues 1000 --outputFile "rand_1000"
        
    

### 1\. OneLockQueue: Coarse-Grained Synchronization \[5 Points\]

For OneLockQueue, we will use a single lock to synchronize across all queue operations. The pseudocode of this solution (without locks) is shown below:

    template<class T>
    struct Node {
        T value
        Node<T>* next
    }
    
    template<class T>
    struct OneLockQueue {
        Node<T>* q_head
        Node<T>* q_tail
        CustomAllocator my_allocator
        
        initQueue(allocator_init) {
            my_allocator.initialize(allocator_init, sizeof(Node<T>))
            node = (Node<T>* )my_allocator.newNode()
            node->next = NULL
            q_head = q_tail = node
        }
    
        enqueue(T value) {
            node = (Node<T>* )my_allocator.newNode()
            node->value = value
            node->next = NULL
            Append to q_tail and update the queue
        }
    
        dequeue(T* p_value) {
            node = q_head
            new_head = q_head->next
            if(new_head == NULL){
                // Queue is empty
                return FALSE
            }
            *p_value = new_head->value
            Update q_head
            my_allocator.freeNode(node)
            return TRUE
        }
    
        cleanup() {
            my_allocator.cleanup();
        }
    }
    

Your task is to implement OneLockQueue using a single lock (coarse grained synchronization). You can assume that `initQueue()` and `cleanup()` will be called by a single thread only.

### 2\. TwoLockQueue: Fine-Grained Synchronization \[5 Points\]

For TwoLockQueue, we will use two separate locks for enqueue and dequeue operations. These locks will ensure that at most one enqueuer and one dequeuer can proceed simultaneously. We will use the two-lock queue algorithm proposed by Maged M. Michael and Michael L. Scott (see `Figure 2` in [paper](https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf)). The pseudocode for this is the same as OneLockQueue but with separate locks for enqueue and dequeue.

        enqueue(T value) {
            // Use enqueue/tail lock
            // Perform enqueue similar to OneLockQueue 
        }
        dequeue(T* p_value) {
            // Use dequeue/head lock
            // Perform dequeue similar to OneLockQueue
        }
    

### 3\. Non-Blocking Queue (Michael-Scott Queue) \[10 Points\]

We will implement the non-blocking queue algorithm proposed by Maged M. Michael and Michael L. Scott (see `Figure 1` in [paper](https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf)). The detailed algorithm is shown below.

    template<class P>
    struct pointer_t {
        P* ptr
    
        P* address(){
            Get the address by getting the 48 least significant bits of ptr
        }
        uint count(){
            Get the count from the 16 most significant bits of ptr
        }
    }
    
    template<class T>
    struct Node {
        T value
        pointer_t<Node<T>> next
    }
    
    template<class T>
    struct NonBlockingQueue {
        pointer_t<Node<T>> q_head
        pointer_t<Node<T>> q_tail
        CustomAllocator my_allocator
        
        initQueue(allocator_init) {
            my_allocator.initialize(allocator_init, sizeof(Node<T>))
            node = (Node<T>* )my_allocator.newNode()
            node->next.ptr = NULL
            q_head.ptr = q_tail.ptr = node
        }
    
        enqueue(T value) {
            node = (Node<T>* )my_allocator.newNode()
            node->value = value
            node->next.ptr = NULL
            SFENCE;
            while(true) {
                tail = q_tail
                LFENCE;
                next = tail.address()->next
                LFENCE;
                if (tail == q_tail){
                    if (next.address() == NULL) {
                        if(CAS(&tail.address()->next, next, <node, next.count()+1>))
                            break
                    }
                    else
                        CAS(&q_tail, tail, <next.address(), tail.count()+1>)	// ELABEL
                }
            }
            SFENCE;
            CAS(&q_tail, tail, <node, tail.count()+1>)
        }
    
        dequeue(T* p_value) {
            while(true){
                head = q_head
                LFENCE;
                tail = q_tail
                LFENCE;
                next = head.address()->next
                LFENCE;
                if (head == q_head) {
                    if(head.address() == tail.address()) {
                        if(next.address() == NULL)
                                return FALSE;
                        CAS(&q_tail, tail, <next.address(), tail.count()+1>)	//DLABEL
                    }
                    else {
                        *p_value = next.address()->value
                        if(CAS(&q_head, head, <next.address(), head.count()+1>))
                            break
                    }
                }
            }
            my_allocator.freeNode(head.address())
            return TRUE
        }
        cleanup() {
            my_allocator.cleanup();
        }
    }
    

**Key things to note in the above algorithm:**

1.  The structure of pointer only has `ptr` as its member variable. Modern 64-bit processors currently support 48-bit virtual addressing. So `count` is co-located within this same `ptr` variable such that the most significant 16 bits are used to store the count. This strategy enables us to update both the address and the count in a single CAS operation.
2.  Notice the use of `SFENCE` and `LFENCE` in the above algorithm. These are assembly instructions defined as follows:
    
        #define LFENCE asm volatile("lfence" : : : "memory")
        #define SFENCE asm volatile("sfence" : : : "memory")
        
    
    We need to use these `fence instructions` to ensure that instructions are not reordered by the compiler or the CPU. For example, the three instructions (`head = q_head` , `tail = q_tail` , `next = head.address()->next`) in the below pseudocode need to happen in the exact given order and we ensure this by inserting `lfence` between them:
    
        head = q_head
        LFENCE;
        tail = q_tail
        LFENCE;
        next = head.address()->next
        LFENCE;
        
    

*   `LFENCE` - Ensures that load instructions are not reordered across the fence instruction.
    
*   `SFENCE` - Ensures that store instructions are not reordered across the fence instruction.
    
    Read more about fence instructions [here](https://en.wikipedia.org/wiki/Memory_barrier). Note that you don't have to insert any other fence instructions apart from the ones that are explicitly shown in the provided pseudocode.
    

3.  You can implement the non-blocking queues with or without `std::atomic` variables. In case you'd like to work without `std::atomic`, you can use the `CAS()` function provided in `common/utils.h`.

### Submission Guidelines

*   Make sure that your solutions folder has the following files and sub-folders. Let's say your solutions folder is called `my_assignment5_solutions`. It should contain:
    *   `common/` -- The folder containing all common files. It is already available in the assignment 5 package. Do not modify it or remove any files.
    *   `lib/` -- The folder containing the library. It is already available in the assignment 5 package. Do not modify it or remove any files.
    *   `queues/one_lock_queue.h`
    *   `queues/two_lock_queue.h`
    *   `queues/non_blocking_queue.h`
    *   `Makefile` -- Makefile for the project. Do not modify this file.
    *   `driver_correctness.cpp` -- You are already provided this file. Do not change it.
    *   `driver_throughput.cpp` -- You are already provided this file. Do not change it.
*   To create the submission file, follow the steps below:
    1.  Enter in your solutions folder, and remove all the object/temporary files.
        
            $ cd my_assignment5_solutions/
            $ make clean
            
        
    2.  Create the tar.gz file.
        
            $ tar cvzf assignment5.tar.gz *
            
        
        which creates a compressed tar ball that contains the contents of the folder.
    3.  Validate the tar ball using the `submission_validator.pyc` script.
        
            $ python /scratch/assignment5/test_scripts/submission_validator.pyc --tarPath=assignment5.tar.gz
            
        
*   For assignment report,
    *   Create a copy of [Assignment 5 - Report](https://docs.google.com/document/d/1giEGNdJDU8YdgMvje6HSdOGUVnlFun50MaX6XLSLgbY/edit?usp=sharing).
    *   Fill in your answers.
    *   Select `File -> Download -> PDF Document`. Save the downloaded file as `report.pdf`.
*   Submit via [CourSys](https://courses.cs.sfu.ca/) by the deadline posted there.

* * *

Copyright © 2020 Keval Vora. All rights reserved.
Created by Group 31:
- abuka032 (Abdulwahid Abukar)
    - Implemented send_file_to_server 
    - assisted with init
    - assisted with accept_connection 
    - assisted with get_file_to_client 
- phimp003 (Jimmy Phimpadabsee)
    - added reusable ports
    - assisted with init (server_addr)
    - wrote setup_connect
    - wrote receive_file_from_server
- krant115 (Logan Krant)
    - Debugged accept_connection to function with all dispatcher threads
    - Debugged file writing in receive_file_from_server (added Makefile to fix)
    - Implemented get_request_from_server
    - Implemented send_file_to_client

Code tested on csel-kh1260-03.cselabs.umn.edu

Changes to existing files: Added output/img directory creation to Makefile

Assumptions other than utils.c documentation: None

How could you enable your program to make EACH individual request parallelized? (high-level pseudocode would be acceptable/preferred for this part)

We can enable our program to parallelize requests by maintaining a pool of threads which constantly run according to the thread system on all the CPU's cores and pick up available requests when they run via a queue; ensuring request are fulfilled and that more CPU time is used fulfilling requests. We let other threads know when a thread has produced or added content in the queue and signal them to run time soon if they are waiting for a space or a request to take; encouraging the thread system to run threads with actual work to do across the CPU cores.

lock l;
request_queue q;

dispatcher():
    while(1):
        get client request
        lock(l)
        while (queue_full(q)):
            wait for queue to have free space
        enqueue(q, client_request)
        let any waiting threads know there's a request in queue
        unlock(l);

worker():
    while(1):
        lock(l)
        while (queue_empty(q)):
            wait for queue to have request(s)
        request = dequeue(q)
        let any waiting threads know there's a free slot in queue
        unlock(l)
        do work on request

main():
    create n dispatcher threads
    create m worker threads
    join threads when done

System Design and Thread Management

Transitioned from on-demand thread creation to a persistent thread pool model for both dispatcher and worker threads, initializing this pool at server startup to optimize resource management and processing efficiency.

Thread Pool Usage: Threads can now manange to handle incoming requests, which significantly reducing the overhead of thread creation and destruction.

Queue Management: Implemented a thread-safe queue controlled via mutex locks and condition variables to manage the task flow between dispatcher and worker threads efficiently.

Mutex Locks: Ensured exclusive access to the queue, which prevents concurrent access issues.

Condition Variables: Facilitated synchronization between dispatcher and worker threads; one variable signals the availability of new tasks, while another indicates free space in the queue for incoming requests.

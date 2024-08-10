/* Filename: jobExecutorServerProcess.h */

#pragma once

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "waitingBufferQueue.h"

typedef unsigned int port_num_t;

namespace Application_Job_Executor_Server {

    /**
     * @brief Public class that represents a Job Executor Server Process. It contains all
     * the basic and appropriate data of the server, port number, buffer size, thread pool size 
     * and implements the necessary methods to receive and send data.
     * 
     * @author Antonis Zikas sdi21000388
    */
    class Process {

    private:

        static port_num_t portNum;          // The port number of the server
        static unsigned int bufferSize;     // The size of the buffer
        static unsigned int threadPoolSize; // the size of the thread pool
        static unsigned int concurrency;    // The concurrency level of the server (how many jobs can run at the same time)

        static int server_fd;               // The server file descriptor, result from listen()
        static struct sockaddr_in address;  //  The address of the server

        static unsigned int runningJobs; // The amount of running jobs at any moment
        static unsigned int busyWorkers; // The amount of busy workers at any moment

        static pid_t processID; // Process ID

        /**
         * @brief Controller Thread function of the server. It creates a new Controller Thread object
         * and executes the basic algorithm of a Controller Thread.
         * 
         * @param socket_desc the client socket descriptor
         * 
         * @return anything
        */
        static void* ControllerThread(void* socket_desc);
    
        /**
         * @brief Worker Thread function of the server. It creates a new Worker Thread object
         * and executes the basic algorithm of a Worker Thread.
         * 
         * @param socket_desc the client socket descriptor
         * 
         * @return anything
        */
        static void* WorkerThread(void* socket_desc);

    public:
        
        /* Supporting flags */
        static bool shouldStop;        // When the server should stop and terminate
        static bool continueExecution; // When the server can continue executing in the while loop

        /* Mutexes */
        static pthread_mutex_t mutex_controller;     // Used for the controller thread of the server
        static pthread_mutex_t mutex_worker;         // Used for the worker thread of the server
        static pthread_mutex_t mutex_jobInsertion;   // Used for jobs insertions in the queue
        static pthread_mutex_t mutex_serverContinue; // Used for server termination
        static pthread_mutex_t mutex_allJobsDone;    // Used to determin when all jobs are done

        /* Condition Variables */
        static pthread_cond_t condVar_controller;     // Used for the controller thread synchronization
        static pthread_cond_t condVar_worker;         // Used for the worker thread synchronization
        static pthread_cond_t condVar_serverContinue; // Used for the server termination synchronization
        static pthread_cond_t condVar_allJobsDone;    // Used to determin when all jobs are done

        /**
         * @brief Initializer of the Job Executor Server Process. Works like a constructor and initializes 
         * the appropriate data of the server, the port number, the buffer size and the thread pool size.
         * 
         * @param portNum the port number of the server
         * @param bufferSize the size of the buffer
         * @param threadPoolSize the size of the thread pool
        */
        static void init(const port_num_t portNum, const unsigned int bufferSize, const unsigned int threadPoolSize);

        /**
         * @brief Destroyer of the Job Executor Server Process. Works like a destructor and deletes any memory 
         * used in the application and destroys the mutexes and condition variables  used for controller threads 
         * and worker threads.
        */
        static void destroy(void);

        /**
         * @brief Returns the amount of running jobs of the server at that moment.
         * 
         * @return the number of jobs running
        */
        static unsigned int getRunningJobs(void);

        /**
         * @brief Returns the amount of busy workers of the server at that time
         * 
         * @return the amount of workers that are busy executing a job
        */
        static unsigned int getBusyWorkers(void);

        /**
         * @brief Returns the concurrency of the server (how many jobs can run at the same time)
         * 
         * @return the current concurrency level
        */
        static unsigned int getConcurrency(void);
        
        /**
         * @brief Increases the amount of running jobs by one.
        */
        static void increaseRunningJobs(void);

        /**
         * @brief Sets the concurrency of the server (how many jobs can run at the same time)
         * 
         * @param concurrency the concurrency level to set
        */
        static void setConcurrency(const unsigned int concurrency);

        /**
         * @brief Creates a new socket of the server and sets its option to reuse the address, so that
         * we don't have to wait until we can run again the server. It also initializes the appropriate data
         * for the server address.
         * 
         * @return true if the initialization was sucessfull, false otherwise
        */
        static bool createSocket(void);

        /**
         * @brief Attaches the new socket to the port of the server. It binds the socket to the server 
         * address and sets it to listen on the port.
         * 
         * @return true if the attaching was successfull, false otherwise
        */
        static bool attachSocketToPort(void);

        /**
         * @brief Runs the server. It waits for new connections to accept them, reads the data that
         * were sent and also sends data back to the client.
         * 
         * @return true if the server ran successfully, false if something was occured
        */
        static bool run(void);

    };


}

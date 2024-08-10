/* Filename: server.cpp */

#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cerrno>
#include <sys/stat.h>
#include "../../include/jobExecutorServerProcess.h"
#include "../../include/controllerThread.h"
#include "../../include/workerThread.h"

/* namespace alias */
namespace Server = Application_Job_Executor_Server;
namespace Controller = Application_Job_Executor_Server::Application_Controller_Thread;
namespace Worker = Application_Job_Executor_Server::Application_Worker_Thread;
namespace WaitingBuffer = Application_Job_Executor_Server::Application_Common_Waiting_Buffer;

/* Declare static variables */
port_num_t Server::Process::portNum;
unsigned int Server::Process::bufferSize;
unsigned int Server::Process::threadPoolSize;

int Server::Process::server_fd;
struct sockaddr_in Server::Process::address;

unsigned int Server::Process::runningJobs = 0;
unsigned int Server::Process::busyWorkers = 0;

pid_t Server::Process::processID;

unsigned int Server::Process::concurrency = 1;
bool Server::Process::shouldStop = false;
bool Server::Process::continueExecution;

pthread_mutex_t Server::Process::mutex_controller;
pthread_mutex_t Server::Process::mutex_worker;
pthread_mutex_t Server::Process::mutex_jobInsertion;
pthread_mutex_t Server::Process::mutex_serverContinue;
pthread_mutex_t Server::Process::mutex_allJobsDone;

pthread_cond_t Server::Process::condVar_controller;
pthread_cond_t Server::Process::condVar_worker;
pthread_cond_t Server::Process::condVar_serverContinue;
pthread_cond_t Server::Process::condVar_allJobsDone;

/**
 * @brief Removes the given directory with all the files containing the outputs of the jobs executed
 * by the worker thread of the server.
 * 
 * @param directoryPath the path of the directory to remove
 * 
 * @return true if the directory was removed successfully, false otherwise
*/
static bool removeTeporaryDirectory(const std::string& directoryPath) {

    // Open the given directory and check if any error has occured
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr) {
        perror("Error opening temporary directory");
        return false;
    }

    // Create a dirent object to iterate through the directory and get every file
    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) 
    {
        // Of course exlude the current and previous directories
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") {
            continue;
        }

        std::string entryPath = directoryPath + "/" + entryName; // Get the file path

        // Remove that file path and check if any error has occured
        if (unlink(entryPath.c_str()) == -1) {
            perror("Error removing temporary file");
            closedir(dir);
            return false;
        }
    }

    closedir(dir); // Close the directory

    // Remove the actual directory and check if any error has occured
    if (rmdir(directoryPath.c_str()) == -1) {
        perror("Error removing temporary directory path");
        return false;
    }

    return true;

}

/**
 * @brief Initializes all the mutexes of the server.
*/
static void initializeServerMutexes(void) {

    // Initialize the mutexes of the Server
    pthread_mutex_init(&Server::Process::mutex_controller, NULL);
    pthread_mutex_init(&Server::Process::mutex_worker, NULL);
    pthread_mutex_init(&Server::Process::mutex_jobInsertion, NULL);
    pthread_mutex_init(&Server::Process::mutex_serverContinue, NULL);
    pthread_mutex_init(&Server::Process::mutex_allJobsDone, NULL);

}

/**
 * @brief Initializes all the condition variables of the server.
*/
static void initializeServerConditionVariables(void) {

    // Initialize the condition variables of the Server
    pthread_cond_init(&Server::Process::condVar_controller, NULL);
    pthread_cond_init(&Server::Process::condVar_worker, NULL);
    pthread_cond_init(&Server::Process::condVar_serverContinue, NULL);
    pthread_cond_init(&Server::Process::condVar_allJobsDone, NULL);

}

/**
 * @brief Destroys all the mutexes of the server.
*/
static void deleteServerMutexes(void) {

    // Destroy the mutexes of the server
    pthread_mutex_destroy(&Server::Process::mutex_controller);
    pthread_mutex_destroy(&Server::Process::mutex_worker);
    pthread_mutex_destroy(&Server::Process::mutex_jobInsertion);
    pthread_mutex_destroy(&Server::Process::mutex_serverContinue);
    pthread_mutex_destroy(&Server::Process::mutex_allJobsDone);
    
}   

/**
 * @brief Destroys all the condition variables of the server.
*/
static void deleteServerConditionVariables(void) {

    // Destroy the condition variables of the Server
    pthread_cond_destroy(&Server::Process::condVar_controller);
    pthread_cond_destroy(&Server::Process::condVar_worker); 
    pthread_cond_destroy(&Server::Process::condVar_serverContinue);
    pthread_cond_destroy(&Server::Process::condVar_allJobsDone  ); 
    
}

/**
 * @brief Initializer of the Job Executor Server Process. Works like a constructor and initializes 
 * the appropriate data of the server, the port number, the buffer size and the thread pool size.
 * 
 * @param portNum the port number of the server
 * @param bufferSize the size of the buffer
 * @param threadPoolSize the size of the thread pool
*/
void Server::Process::init(const port_num_t portNum, const unsigned int bufferSize, const unsigned int threadPoolSize) {

    // Initialize the basic data of the server
    Server::Process::portNum = portNum; Server::Process::bufferSize = bufferSize; Server::Process::threadPoolSize = threadPoolSize;

    // Assign the process ID and set the capacity of the buffer queue
    Server::Process::processID = getpid();
    WaitingBuffer::Queue::setCapacity(Server::Process::bufferSize);

    // Initialize mutexes and condition variables
    initializeServerMutexes();
    initializeServerConditionVariables();

}

/**
 * @brief Destroyer of the Job Executor Server Process. Works like a destructor and deletes any memory 
 * used in the application and destroys the mutexes and condition variables  used for controller threads 
 * and worker threads.
*/
void Server::Process::destroy(void) {

    // Destroy the mutexes and condition variables of the server
    deleteServerMutexes();  
    deleteServerConditionVariables();

}

/**
 * @brief Returns the amount of running jobs of the server at that moment.
 * 
 * @return the number of jobs running
*/
unsigned int Server::Process::getRunningJobs(void) {
    return Server::Process::runningJobs;
}

/**
 * @brief Returns the amount of busy workers of the server at that time
 * 
 * @return the amount of workers that are busy executing a job
*/
unsigned int Server::Process::getBusyWorkers(void) {
    return Server::Process::busyWorkers;
}

/**
 * @brief Returns the concurrency of the server (how many jobs can run at the same time)
 * 
 * @return the current concurrency level
*/
unsigned int Server::Process::getConcurrency(void) {
    return Server::Process::concurrency;
}

/**
 * @brief Increases the amount of running jobs by one.
*/
void Server::Process::increaseRunningJobs(void) {
    Server::Process::runningJobs++;
}

/**
 * @brief Sets the concurrency of the server (how many jobs can run at the same time)
 * 
 * @param concurrency the concurrency level to set
*/
void Server::Process::setConcurrency(const unsigned int concurrency) {
    Server::Process::concurrency = concurrency;
}

/**
 * @brief Creates a new socket of the server and sets its option to reuse the address, so that
 * we don't have to wait until we can run again the server. It also initializes the appropriate data
 * for the server address.
 * 
 * @return true if the initialization was sucessfull, false otherwise
*/
bool Server::Process::createSocket(void) {

    // Create a socket file descriptor
    if ((Server::Process::server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creating server socket");
        return false;
    }

    // Set the socket option to reuse the address
    int opt = 1;
    if (setsockopt(Server::Process::server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Error setting the socket option to reuse the address");
        close(Server::Process::server_fd);
        return false;
    }

    // Build the address of the server
    Server::Process::address.sin_family = AF_INET;
    Server::Process::address.sin_addr.s_addr = INADDR_ANY;
    Server::Process::address.sin_port = htons(Server::Process::portNum);

    return true;
}

/**
 * @brief Attaches the new socket to the port of the server. It binds the socket to the server 
 * address and sets it to listen on the port.
 * 
 * @return true if the attaching was successfull, false otherwise
*/
bool Server::Process::attachSocketToPort(void) {

    // Bind the socket
    if (bind(Server::Process::server_fd, (struct sockaddr*)&Server::Process::address, sizeof(Server::Process::address)) < 0) {
        perror("Error binding the socket");
        close(Server::Process::server_fd);
        return false;
    }

    // Listen on port
    if (listen(Server::Process::server_fd, 3) < 0) {
        perror("Error listening to port");
        close(Server::Process::server_fd);
        return false;
    }

    return true;
}

/**
 * @brief Runs the server. It waits for new connections to accept them, reads the data that
 * were sent and also sends data back to the client.
 * 
 * @return true if the server ran successfully, false if something was occured
*/
bool Server::Process::run(void) {
    
    int addrlen = sizeof(Server::Process::address);
    int client_socket;

    pthread_t worker_threads[Server::Process::threadPoolSize];

    // Create the worker threads
    for (unsigned int i = 0; i < Server::Process::threadPoolSize; i++) {
        if (pthread_create(&worker_threads[i], NULL, Server::Process::WorkerThread, NULL) != 0) {
            perror("Error creating worker thread");
            return false;
        }
    }

    // Server listening on port loop
    while(!Server::Process::shouldStop) 
    {
        // Accept connections from clients
        if ((client_socket = accept(Server::Process::server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept Failed");
            close(Server::Process::server_fd);
            return false;
        }
        
        pthread_t clientThread;
        Server::Process::continueExecution = false;
        if (pthread_create(&clientThread, NULL, Server::Process::ControllerThread, (void*)&client_socket) != 0) {
            perror("Error creating controller thread");
            return false;
        }

        pthread_detach(clientThread);

        // Wait until the server can continue executing
        pthread_mutex_lock(&Server::Process::mutex_serverContinue);
        while (!Server::Process::continueExecution) {
            pthread_cond_wait(&Server::Process::condVar_serverContinue, &Server::Process::mutex_serverContinue);
        }
        pthread_mutex_unlock(&Server::Process::mutex_serverContinue);
    }

    close(Server::Process::server_fd);

    // Notify worker threads to exit
    pthread_mutex_lock(&Server::Process::mutex_worker);
    pthread_cond_broadcast(&Server::Process::condVar_worker);
    pthread_mutex_unlock(&Server::Process::mutex_worker);

    // Join worker threads to ensure proper cleanup
    for (unsigned int i = 0; i < Server::Process::threadPoolSize; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    // Delete the temporary directory of all the output files
    if (!removeTeporaryDirectory("temp")) {
        return false;
    }
    
    return true;
}

/**
 * @brief Controller Thread function of the server. It creates a new Controller Thread object
 * and executes the basic algorithm of a Controller Thread.
 * 
 * @param args the arguments of the function
 * 
 * @return anything
*/
void* Server::Process::ControllerThread(void* socket_desc) {

    int clientSocket = *(int*)socket_desc;

    Controller::Thread thread = Controller::Thread(clientSocket);
    thread.receiveClientCommandFromSocket();
    thread.executeTask();

    return nullptr;

}

/**
 * @brief Worker Thread function of the server. It creates a new Worker Thread object
 * and executes the basic algorithm of a Worker Thread.
 * 
 * @param socket_desc the client socket descriptor
 * 
 * @return anything
*/
void* Server::Process::WorkerThread(void* arg) {

    bool shouldStop = false;

    // Main Loop of the Worker Thread
    while (true) {

        pthread_mutex_lock(&Server::Process::mutex_worker);
        while (WaitingBuffer::Queue::isEmpty() || Server::Process::runningJobs == Server::Process::concurrency) {
            pthread_cond_wait(&Server::Process::condVar_worker, &Server::Process::mutex_worker);
            
            // Check if the server should stop
            if (Server::Process::shouldStop) {
                pthread_mutex_unlock(&Server::Process::mutex_worker);
                shouldStop = true;
                break;
            }
        }

        pthread_mutex_unlock(&Server::Process::mutex_worker);

        // Check if the server should terminate before creating a Worker Thread object
        if (shouldStop) { break; } 

        Worker::Thread workerThread = Worker::Thread();

        pthread_mutex_lock(&Server::Process::mutex_jobInsertion);
        CC::JobTriplate triplate = workerThread.receiveJobFromBuffer();
        pthread_mutex_unlock(&Server::Process::mutex_jobInsertion);

        pthread_mutex_lock(&Server::Process::mutex_worker);
        pthread_cond_signal(&Server::Process::condVar_controller);
        pthread_mutex_unlock(&Server::Process::mutex_worker);
        
        Server::Process::busyWorkers++;
        workerThread.executeJob(triplate);

        pthread_mutex_lock(&Server::Process::mutex_worker);
        Server::Process::runningJobs--;
        Server::Process::busyWorkers--;
        pthread_mutex_unlock(&Server::Process::mutex_worker);

        pthread_cond_signal(&Server::Process::condVar_allJobsDone);
    }

    return nullptr;

}

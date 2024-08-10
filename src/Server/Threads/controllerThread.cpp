/* Filename: controllerThread.cpp */

#include <string.h>
#include "../../../include/common.h"
#include "../../../include/controllerThread.h"
#include "../../../include/waitingBufferQueue.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/* namespace alias */
namespace Server = Application_Job_Executor_Server;
namespace Controller = Application_Job_Executor_Server::Application_Controller_Thread;
namespace WaitingBuffer = Application_Job_Executor_Server::Application_Common_Waiting_Buffer;

/* Static variables initialization */
unsigned int Controller::Thread::jobsEntered = 0; // Initialize the number of jobs entered
bool Controller::Thread::shouldStop = false;

/**
 * @brief Constructor of the Controller Thread. It stores the socket of the client
 * that is being used for communication with the client.
 * 
 * @param clientSocket the socket id of the client
*/
Controller::Thread::Thread(const int clientSocket) {

    this->clientSocket = clientSocket;

}

/**
 * @brief Receives the command that a client has sent to the server and returns
 * that command as a C++ string.
 * 
 * @return true if the command was received successfully, false otherwise
*/
bool Controller::Thread::receiveClientCommandFromSocket(void) {

    // Create variables that will hold the message and its size
    ssize_t commandSize;
    char* command;

    // First read the command size
    if (read(this->clientSocket, &commandSize, sizeof(ssize_t)) == -1) {
        perror("Error receiving client command size");
        return false;
    }

    // Allocate memory for the command
    command = new char[commandSize + 1];
    if (command == nullptr) {
        perror("Error Allocating memory");
        return false;
    }
    
    // Then read the actual command
    if (read(this->clientSocket, command, commandSize) == -1) {
        perror("Error receiving client command");
        return false;
    }

    // Initialize the client command of the controller thread and determin its mode
    command[commandSize] = '\0';
    this->clientCommand = command;
    this->clientCommandMode = getClientCommandMode(this->clientCommand);

    delete[] command;
    
    return true;
}

/**
 * @brief Executes the appropriate task of the controller thread, according to
 * the client command received and its mode.
 * 
 * @return true if the task was done successfully, false otherwise 
*/
bool Controller::Thread::executeTask(void) {

    // Determin the task mode and call the appropriate function for this task
    switch (this->clientCommandMode) {

        case CC::JECC_ISSUE_JOB: this->insertNewJobToBufferQueue(); break;
        case CC::JECC_SET_CONCURRENCY: this->setServerConcurrencyLevel(); break;
        case CC::JECC_POLL: this->sendWaitingJobsToClient(); break;
        case CC::JECC_STOP: this->removeJobFromBufferQueue(); break;
        case CC::JECC_EXIT: this->terminateServer(); break;
        default: break;
    
    }

    return true;
}

/**
 * @brief Handles the issueJob client command. It receives the full command of the
 * client, determines what the bash job is and puts it to the common queue buffer
 * to be executed later on by a worker thread.
 * 
 * @return true, if the process was successfull, false otherwise
*/
bool Controller::Thread::insertNewJobToBufferQueue(void) {

    pthread_mutex_lock(&Server::Process::mutex_serverContinue);
    Server::Process::continueExecution = true;
    pthread_cond_signal(&Server::Process::condVar_serverContinue);
    pthread_mutex_unlock(&Server::Process::mutex_serverContinue);

    pthread_mutex_lock(&Server::Process::mutex_controller);

    // If the waiting queue is full, the controller thread must wait until a job is removed
    while (WaitingBuffer::Queue::isFull()) {
        
        pthread_cond_wait(&Server::Process::condVar_controller, &Server::Process::mutex_controller);

        // If the server should stop notify the client that the job was not placed in the queue, due to server termination
        if (Controller::Thread::shouldStop) 
        {
            char notificationMessage[] = "JOB SUBMIT CANCELED BECAUSE OF SERVER TERMINATION";
            ssize_t notificationSize = strlen(notificationMessage);

            send(this->clientSocket, &notificationSize, sizeof(ssize_t), 0);
            send(this->clientSocket, notificationMessage, notificationSize, 0);

            pthread_mutex_unlock(&Server::Process::mutex_controller);

            return true;
        }
    
    }

    pthread_mutex_unlock(&Server::Process::mutex_controller);

    // Initialize the appropriate data for a new job triplate
    std::string job = removeFirstWord(this->clientCommand);
    std::string jobID = "job_" + std::to_string(++Controller::Thread::jobsEntered);
    int socket_ID = this->clientSocket;

    // Create the new job triplate of the new command and insert it to the waiting buffer queue
    CC::JobTriplate newJobTriplate = { jobID, job, socket_ID };

    pthread_mutex_lock(&Server::Process::mutex_jobInsertion);
    WaitingBuffer::Queue::insertJobTriplate(newJobTriplate);
    pthread_mutex_unlock(&Server::Process::mutex_jobInsertion);

    // Set up server response for the client
    std::string serverResponse = "JOB <" + jobID + ", " + job + "> SUBMITTED";
    const char* response = serverResponse.c_str();
    ssize_t responseSize = strlen(response);

    // Send the response back to the client
    send(socket_ID, &responseSize, sizeof(ssize_t), 0);
    send(socket_ID, response, responseSize, 0);

    std::cout << "---[" << KCYN << "New Job Submittion" << KWHT << "]--- | ";
    std::cout << KCYN << "Controller Thread has submitted a new job" << KWHT << " | ";
    std::cout <<  "Job ID: " << "[" << KGRN << newJobTriplate.jobID << KWHT << "]" << " | ";
    std::cout <<  "Job command: " << "'" << KBLU << newJobTriplate.job << KWHT << "'" << " | ";
    std::cout <<  "Socket ID: " << "[" << KRED << newJobTriplate.socketID << KWHT << "]";
    std::cout << std::endl;

    pthread_cond_signal(&Server::Process::condVar_worker); // Notify that a job has been placed in the queue

    return true;

}

/**
 * @brief Handles the setConcurrency client command. It determines what the given 
 * concurrency is and sets it as the new one in the application. 
 * 
 * @return true, if the process was successfull, false otherwise
*/
bool Controller::Thread::setServerConcurrencyLevel(void) {

    pthread_mutex_lock(&Server::Process::mutex_serverContinue);
    Server::Process::continueExecution = true;
    pthread_cond_signal(&Server::Process::condVar_serverContinue);
    pthread_mutex_unlock(&Server::Process::mutex_serverContinue);

    // Receive the concurrency that was sent by the client
    unsigned int oldConcurrency = Server::Process::getConcurrency();
    std::string newConcurrensy = removeFirstWord(this->clientCommand);
    Server::Process::setConcurrency(stoi(newConcurrensy));

    // Build the appropriate response
    std::string message = "CONCURRENCY SET AT " + newConcurrensy;
    const char* serverResponse = message.c_str();
    ssize_t serverResponseSize = strlen(serverResponse);

    // Send the response
    send(this->clientSocket, &serverResponseSize, sizeof(ssize_t), 0);
    send(this->clientSocket, serverResponse, serverResponseSize, 0);

    // Wake up a worker threads to pick a job
    for (int i = 0; i < stoi(newConcurrensy) - (int)Server::Process::getBusyWorkers(); i++) {
        pthread_mutex_lock(&Server::Process::mutex_worker);
        pthread_cond_signal(&Server::Process::condVar_worker);
        pthread_mutex_unlock(&Server::Process::mutex_worker);
    }

    std::cout << "---[" << KMAG << "Concurrency Change" << KWHT << "]--- | ";
    std::cout << "Old: " << "[" << KRED << oldConcurrency << KWHT << "]" << " | ";
    std::cout << "New: " << "[" << KGRN << newConcurrensy << KWHT << "]" << std::endl;

    return true;

}

/**
 * @brief Handles the poll client command. It iterates through the waiting buffer
 * queue, and sends each individual job triplate back to the client.
 * 
 * @return true, if the process was successfull, false otherwise
*/
bool Controller::Thread::sendWaitingJobsToClient(void) {

    pthread_mutex_lock(&Server::Process::mutex_serverContinue);
    Server::Process::continueExecution = true;
    pthread_cond_signal(&Server::Process::condVar_serverContinue);
    pthread_mutex_unlock(&Server::Process::mutex_serverContinue);

    // Select and send the number of jobs waiting to the client, to let it know how manu jobs to receive
    ssize_t bufferSize = WaitingBuffer::Queue::getSize();
    send(this->clientSocket, &bufferSize, sizeof(ssize_t), 0);

    // Iterate through the waiting buffer queue, select every job triplate and send it to the client
    for (unsigned int i = 0; i < bufferSize; i++) {

        CC::JobTriplate triplate = WaitingBuffer::Queue::at(i);
        std::string currentMessage = triplate.job + ", " + triplate.jobID;
        
        const char* message = currentMessage.c_str();
        ssize_t messageSize = strlen(message);

        send(this->clientSocket, &messageSize, sizeof(ssize_t), 0);
        send(this->clientSocket, message, messageSize, 0);

    }

    return true;

}

/**
 * @brief Handles the stop client command. It itrates through the waiting buffer
 * queue, until it finds the job specified to be removed from the buffer, according
 * to its job ID.
 * 
 * @return true, if the process was successfull, false otherwise
*/
bool Controller::Thread::removeJobFromBufferQueue(void) {

    CC::JobTriplate triplate;

    pthread_mutex_lock(&Server::Process::mutex_serverContinue);
    Server::Process::continueExecution = true;
    pthread_cond_signal(&Server::Process::condVar_serverContinue);
    pthread_mutex_unlock(&Server::Process::mutex_serverContinue);

    // Receive the job ID of the job to remove from the waiting buffer queue
    std::string message;
    std::string given_job_ID = removeFirstWord(this->clientCommand);

    bool found = WaitingBuffer::Queue::removeJobTriplateByID(given_job_ID, triplate); // Remove the job

    // Build the appropriate response
    if (found) { message = "JOB " + given_job_ID + " REMOVED"; } else { message = "JOB " + given_job_ID + " NOTFOUND"; }

    const char* serverResponse = message.c_str();
    ssize_t serverResponseSize = strlen(serverResponse);

    // Send the response to the client
    send(this->clientSocket, &serverResponseSize, sizeof(ssize_t), 0);
    send(this->clientSocket, serverResponse, serverResponseSize, 0);

    pthread_mutex_lock(&Server::Process::mutex_controller);

    if (found) {

        pthread_cond_signal(&Server::Process::condVar_controller);

        // Send an appropriate message to the client of the triplate saying that the job has been stopped
        char notificationMessage[] = "JOB HAS BEEN REMOVED BEFORE EXECUTION";
        ssize_t notificationSize = strlen(notificationMessage);

        send(triplate.socketID, &notificationSize, sizeof(ssize_t), 0);
        send(triplate.socketID, notificationMessage, notificationSize, 0);
    
    }

    pthread_mutex_unlock(&Server::Process::mutex_controller);

    return true;

}

/**
 * @brief Handles the exit client command. It sends a termination message back to the
 * client and terminates the server.
 * 
 * @return true, if the process was successfull, false otherwise
*/
bool Controller::Thread::terminateServer(void) {

    Controller::Thread::shouldStop = true;

    // Notify all the clients waiting for their job to be submitted, that the server has been terminated
    pthread_mutex_lock(&Server::Process::mutex_controller);
    pthread_cond_broadcast(&Server::Process::condVar_controller);
    pthread_mutex_unlock(&Server::Process::mutex_controller);

    // Remove all the jobs waiting in the buffer queue and notify every client that the server has been terminated
    while (!WaitingBuffer::Queue::isEmpty()) {

        CC::JobTriplate triplate = WaitingBuffer::Queue::getJobTriplate();
        int socketID = triplate.socketID;

        char response[] = "SERVER TERMINATED BEFORE EXECUTION";
        ssize_t responseSize = strlen(response);

        send(socketID, &responseSize, sizeof(ssize_t), 0);
        send(socketID, response, responseSize, 0);

    }
    
    // Wait until no job is running
    pthread_mutex_lock(&Server::Process::mutex_allJobsDone);
    while(Server::Process::getRunningJobs() > 0) {
        pthread_cond_wait(&Server::Process::condVar_allJobsDone, &Server::Process::mutex_allJobsDone);
    }
    pthread_mutex_unlock(&Server::Process::mutex_allJobsDone);

    // Build the appropriate response
    std::string message = "SERVER TERMINATED";

    const char* serverResponse = message.c_str();
    ssize_t serverResponseSize = strlen(serverResponse);

    // Send the response
    send(this->clientSocket, &serverResponseSize, sizeof(ssize_t), 0);
    send(this->clientSocket, serverResponse, serverResponseSize, 0);

    // Finally terminate the server
    Server::Process::shouldStop = true;

    pthread_mutex_lock(&Server::Process::mutex_serverContinue);
    Server::Process::continueExecution = true;
    pthread_cond_signal(&Server::Process::condVar_serverContinue);
    pthread_mutex_unlock(&Server::Process::mutex_serverContinue);

    std::cout << "---[" << KRED << "Server Termination" << KWHT << "]---" << " | ";
    std::cout << KRED << "A client has terminated the server" << KWHT << std::endl;

    return true;

}

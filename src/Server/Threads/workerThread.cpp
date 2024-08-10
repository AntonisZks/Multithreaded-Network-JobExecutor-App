/* Filename: workerThread.cpp */

#include <string.h>
#include <sys/wait.h>
#include "../../../include/workerThread.h"
#include "../../../include/waitingBufferQueue.h"
#include "../../../include/jobExecutorServerProcess.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define PIPE_READ_END  (0)
#define PIPE_WRITE_END (1)

#define MAX_OUTPUT_FILE_PATH (100)

/* namespace alias */
namespace Server = Application_Job_Executor_Server;
namespace Worker = Application_Job_Executor_Server::Application_Worker_Thread;
namespace WaitingBuffer = Application_Job_Executor_Server::Application_Common_Waiting_Buffer;

/**
 * @brief Supporting function to send data from the worker thread to its child process
 * using a pipe, and specifically the the path of the file where the output of the job 
 * executing by the worker thread is going to be placed for later use.
 * 
 * @param pipefd the pipe needed for communication with the child process
 * @param message the message to send
 * 
 * @return true if the message was sent successfully, false otherwise
*/
static bool sendDataToChildProcess(const int pipefd[2], const char message[MAX_OUTPUT_FILE_PATH]) {

    // First close the read end of the pipe as it is useless for now
    close(pipefd[PIPE_READ_END]);

    // Then write the message to the write end of the pipe, and after that close it
    if (write(pipefd[PIPE_WRITE_END], (char*)message, MAX_OUTPUT_FILE_PATH) == -1) {
        perror("Error sending dat to child process");
        return false;
    }
    
    close(pipefd[PIPE_WRITE_END]);

    return true;
}

/**
 * @brief Supporting function that receives the data sent to the child process by the 
 * parent process through a pipe. Specifically it receives the path of the file to
 * contain the output of the job executed by the worker thread of the server.
 * 
 * @param pipefd the pipe used for communication
 * @param message the message to receive
 * 
 * @return true if the data were received successfully, false otherwise
*/
static bool receiveDataFromParentProcess(const int pipefd[2], const char message[MAX_OUTPUT_FILE_PATH]) {

    // First close the write end as it is useless for now
    close(pipefd[PIPE_WRITE_END]);

    // Then read the data from read end of the pipe, and after that close the read end
    if (read(pipefd[PIPE_READ_END], (char*)message, MAX_OUTPUT_FILE_PATH) == -1) {
        perror("Error receiving data from parent process");
        return false;
    }

    close(pipefd[PIPE_READ_END]);

    return true;
}

/**
 * @brief Supporting function to create the file where the output of the job executed
 * by the worker thread of the server, will be placed for later use.
 * 
 * @param filename the name of the file
 * @param fd the file descriptor of the file
 * 
 * @return true if the file creation was successfull, false otherwise
*/
static bool createOutputFile(const char* filename, int& fd) {

    // Create a temporary directory to store the output of the executing job
    if (mkdir("temp", 0777) == -1) {
        if (errno != EEXIST) { 
            perror("Error creating temporary file"); 
            return false;
        }
    }

    // Create the file to save process id and check for errors
    if ((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777)) == -1) {
        perror("Failed to create output file");
        return false;
    }

    return true;

}

/**
 * @brief Supporting function to read a file and store and return its contents and its size.
 * 
 * @param filename the name of the file
 * @param fileSize the size of the file
 * 
 * @return the contents of the file if the process worked successfully, nullptr otherwise
*/
static char* readFile(const char* filename, ssize_t& fileSize) {

    // Open the file for reading and check if any error occurred
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening the file");
        return nullptr;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting filesize");
        close(fd);
        return nullptr;
    }

    ssize_t file_size = st.st_size; // Store the size of the file

    // Allocate memory for the file content
    char* contents = new char[file_size + 1];
    if (contents == nullptr) {
        perror("Error allocating memory");
        close(fd);

        return nullptr;
    }

    // Read the contents of the file in a loop to ensure all data is read
    ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size) 
    {
        ssize_t bytes_read = read(fd, contents + total_bytes_read, file_size - total_bytes_read);
        
        if (bytes_read == -1) {
            perror("Error reading file");
            delete[] contents;
            close(fd);

            return nullptr;
        }

        total_bytes_read += bytes_read;
    }

    contents[total_bytes_read] = '\0';
    close(fd);

    fileSize = total_bytes_read;

    return contents;

}

/**
 * @brief Splits the given string that contaings the command that is about to 
 * be executed, into tokens. Those tokens are the arguments that exec() function 
 * needs to run the executable in the right way. The first argument of those tokens 
 * is the executable itself (its name).
 * 
 * @param job the string containing the full command to run the executable
 * 
 * @return an array of tokens, where each one is a specific argument from the given job string
 */
static char** getJobArguments(const char* job) {

    // Find the number of arguments of the job
    unsigned int argumentsCount = 1;

    for (unsigned int i = 0; job[i] != '\0'; i++) {
        if (job[i] == ' ' && job[i + 1] != ' ') {
            argumentsCount++;
        }
    }

    // Store all the arguments
    char** arguments = new char*[argumentsCount + 1];
    if (arguments == nullptr) {
        perror("Error Allocating Memory");
        return nullptr;
    }

    arguments[0] = strtok((char*)job, " ");
    for (unsigned int i = 1; i < argumentsCount; i++) {
        arguments[i] = strtok(NULL, " ");
    }

    arguments[argumentsCount] = NULL;

    return arguments;

}

/**
 * @brief Receives and returns a job from the waiting buffer queue, in order to
 * be executed. It also initializes the client socket ID from the triplate.
 * 
 * @return the first job triplate of the waiting buffer queue
*/
CC::JobTriplate Worker::Thread::receiveJobFromBuffer(void) {

    // Get the forst job triplate of the buffer queue and return it
    CC::JobTriplate triplate = WaitingBuffer::Queue::getJobTriplate();
    this->clientSocket = triplate.socketID;
    return triplate;

}

/**
 * @brief Sends the output of the job executed by the thread back to the client.
 * 
 * @param output the output of the job
 * @param outputSize the size of the output
 * 
 * @return true if the output was sent successfully, false otherwise
*/
bool Worker::Thread::sendJobOutputToClient(const char* output, const ssize_t outputSize) {

    // Send the size of the output, and then the actual output back to the client
    send(this->clientSocket, &outputSize, sizeof(ssize_t), 0);
    send(this->clientSocket, output, outputSize, 0);

    return true;

}

/**
 * @brief Creates and returns the reponse of the worker thread to the client according
 * to the job output. Specifically it reads the output file of the job and adds an extra
 * text at the beginning and at the end of the message.
 * 
 * @param jobOutput the job output stored in the output file
 * @param jobOutputSize the size of the job output
 * @param jobID the ID of the current job
 * @param responseSize the size of the final response
 * 
 * @return the full response string of the worker thread.
*/
char* Worker::Thread::createOutputResponse(const char* jobOutput, const ssize_t jobOutputSize, const char* jobID, ssize_t& responseSize) {

    char startingText[100], endingText[100];
    char* finalResponse = new char[jobOutputSize + 200]; // Initialize the final response string

    // Add the extra string at the start and at the end
    sprintf(startingText, "-----%s output start------", jobID);
    sprintf(endingText, "-----%s output end------", jobID);
    sprintf(finalResponse, "%s\n%s\n%s", startingText, jobOutput, endingText);

    responseSize = strlen(finalResponse); // Store the size of the response

    return finalResponse;
}

/**
 * @brief Receives a job triplate and executes its corresponding job. It creates 
 * a child process with fork() system call and uses exec() to execute the given
 * process
 * 
 * @param jobTriplate the triplate containing the job to execute
 * 
 * @return true if the job was executed successfully, false otherwise 
*/
bool Worker::Thread::executeJob(const CC::JobTriplate jobTriplate) {

    int pipefd[2];
    pid_t pid;

    std::cout << "---[" << KYEL << "New Job  Execution" << KWHT << "]--- | ";
    std::cout << KYEL << "Worker Thread is executing a job " << KWHT << " | ";
    std::cout <<  "Job ID: " << "[" << KGRN << jobTriplate.jobID << KWHT << "]" << " | ";
    std::cout <<  "Job command: " << "'" << KBLU << jobTriplate.job << KWHT << "'";
    std::cout << std::endl;

    // Create the pipe for communication with the child process and check if and error occured
    if (pipe(pipefd) == -1) {
        perror("Pipe");
        return false;
    }

    // Create a child process and check if any error occured
    if ((pid = fork()) == -1) {
        perror("Error creating child process");
        return false;
    }

     /* Parent process code */
    if (pid > 0) {

        Server::Process::increaseRunningJobs();

        // Construct the path of the file that will contain the output of the job
        char jobOutputFilePath[MAX_OUTPUT_FILE_PATH];
        sprintf(jobOutputFilePath, "temp/%d.output", pid);

        // Send the file path to the child process
        if (!sendDataToChildProcess(pipefd, jobOutputFilePath)) {
            return false;
        }

        wait(NULL); // Wait for child process to finish execution

        std::cout << "---[ " << KGRN << "Job  Termination" << KWHT << " ]---" << " | ";
        std::cout << KGRN << jobTriplate.jobID << " was successfully executed!" << KWHT << std::endl;

        ssize_t contentsSize;
        ssize_t responseSize;
        
        // Build the output response and send it back to the client
        char* contents = readFile(jobOutputFilePath, contentsSize);
        char* outputResponse = this->createOutputResponse(contents, contentsSize, jobTriplate.jobID.c_str(), responseSize);

        this->sendJobOutputToClient(outputResponse, responseSize);

        delete[] outputResponse;

        // Delete the temporary output file of the current job
        if (unlink(jobOutputFilePath) != 0) {
            perror("Error deleting temporary output file");
            return false;
        }
        
    }
    
    /* Child process code */
    else if (pid == 0) {

        int fd;
        char jobOutputFilePath[MAX_OUTPUT_FILE_PATH];

        // Receive the file path from the parent process
        if (!receiveDataFromParentProcess(pipefd, jobOutputFilePath)) {
            exit(EXIT_FAILURE);
        }

        // Create the output file
        if (!createOutputFile(jobOutputFilePath, fd)) {
            exit(EXIT_FAILURE);
        }

        // Get the job executable and its arguments from the job triplate
        const char* jobString = jobTriplate.job.c_str();
        char** jobArguments = getJobArguments(jobString);
        char* jobExecutable = jobArguments[0];

        // Duplicate the STDOUT file descriptor to the output file and execute the job
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execvp(jobExecutable, jobArguments);

    }

    return true;
}

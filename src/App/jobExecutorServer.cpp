/* Filename: jobExecutorServer.cpp */

#include <iostream>
#include "../../include/jobExecutorServerProcess.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

namespace Server = Application_Job_Executor_Server; // namespace alias

typedef unsigned int port_num_t;

static bool getCommandLineArguments(int argc, char** argv, port_num_t& portNum, unsigned int& bufferSize, unsigned int& threadPoolSize);

/**
 * @brief Main Entry Point of the application server. Here the server is being initialized by typing to the tty
 * the following command:
 * 
 * ./bin/jobExecutorServer [portNum] [bufferSize] [threadPoolSize]
 * 
 * @param argc the number of command line arguments
 * @param argv the actual command line arguments
 * 
 * @return program termination code
 * 
 * @author Antonis Zikas sdi2100038 
*/
int main(int argc, char* argv[]) {

    // Create the basic data of the server
    port_num_t portNum;
    unsigned int bufferSize;
    unsigned int threadPoolSize;

    // Initialize these data according to the command line arguments
    if (!getCommandLineArguments(argc, argv, portNum, bufferSize, threadPoolSize)) {
        return 1;
    }
    
    Server::Process::init(portNum, bufferSize, threadPoolSize); // Initialize the application server

    // Create a socket for communication and attach it to the server
    Server::Process::createSocket();
    if (!Server::Process::attachSocketToPort()) {
        return 1;
    }

    // Print out a message and run the server
    std::cout << std::endl << KGRN << "Server is listening on port " << KYEL << portNum << KWHT << "..." << std::endl << std::endl;
    Server::Process::run();

    // After the execution of the server delete it
    Server::Process::destroy();

    return 0;
}

/**
 * @brief Receives the command line arguments the user has given while running the server, then sets and returns 
 * the basic data of the server, according to those arguents.
 * 
 * @param argc the number of command line arguments
 * @param argv the actual command line arguments
 * 
 * @param serverName the name of the server
 * @param portNum the port number of the server
 * @param command the full command of the user
 * 
 * @return true if the data initialization was successful, false otherwise
*/
static bool getCommandLineArguments(int argc, char** argv, port_num_t& portNum, unsigned int& bufferSize, unsigned int& threadPoolSize) {

    // Checking for valid number of arguments
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " [portNum] [bufferSize] [threadPoolSize]" << std::endl;
        return false;
    }

    // Assign the command line data
    portNum = atoi(argv[1]);
    bufferSize = atoi(argv[2]);
    threadPoolSize = atoi(argv[3]);

    return true;

}

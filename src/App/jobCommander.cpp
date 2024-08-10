/* Filename: jobCommander.cpp */

#include <iostream>
#include <string>
#include "../../include/jobCommanderProcess.h"

typedef unsigned int port_num_t;

static bool getCommandLineArguments(int argc, char** argv, std::string& serverName, unsigned int& portNum, std::string& command);

/**
 * @brief Main Entry Point of the client process. Here a client is being started from the user, by typing to its tty
 * the following command:
 * 
 * ./bin/jobCommander [serverName] [portNum] [jobCommanderInputCommand]
 * 
 * @param argc the number of command line arguments
 * @param argv the actual command line arguments
 * 
 * @return program termination code
 * 
 * @author Antonis Zikas sdi2100038 
*/
int main(int argc, char* argv[]) {

    // Create the basic data need for communication with the server
    std::string serverName;
    port_num_t portNum;
    std::string jobCommanderInputCommand;

    // Initialize these data according to the user command line arguements
    if (!getCommandLineArguments(argc, argv, serverName, portNum, jobCommanderInputCommand)) {
        return 1;
    }

    Client::Process::init(serverName, portNum, jobCommanderInputCommand);
    
    if (!Client::Process::getServerIPAddress()) return 2;
    if (!Client::Process::createSocket()) return 3;
    if (!Client::Process::connectToServer()) return 4;
    if (!Client::Process::sendCommand()) return 5;
    if (!Client::Process::receiveServerResponse()) return 6;

    return 0;
}

/**
 * @brief Receives the command line arguments the user has given while running jobCommander, then sets and returns 
 * the basic data needed for communication with the server, according to those arguments.
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
static bool getCommandLineArguments(int argc, char** argv, std::string& serverName, unsigned int& portNum, std::string& command) {

    // Checking for valid number of arguments
    if (argc < 4) { 
        std::cout << "Usage: " << argv[0] << " [serverName] [portNum] [jobCommanderInputCommand]" << std::endl;
        return false;
    }

    // Assign the server name and the port number
    serverName = argv[1];
    portNum = atoi(argv[2]);

    // Receive the full command from the tty and turn it to a full string
    command = "";
    for (int i = 3; i < argc; i++) {
        command += argv[i];
        
        // Add spaces between
        if (i < argc - 1) { command += " "; }
    }

    return true;

}

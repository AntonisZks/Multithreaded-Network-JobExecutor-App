/* Filename: client.cpp */

#include "../../include/jobCommanderProcess.h"
#include "../../include/clientCommands.h"
#include "../../include/communication.h"

/* Namespace alias */
namespace ClientCommunication = Application_Client_Server_Communication::Application_Job_Commander_Client;
namespace CC = Application_Job_Commander_Client::Application_Client_Commands;

/* Declare Static Variables */
std::string Client::Process::serverName;
std::string Client::Process::command;
port_num_t Client::Process::portNum;;
std::string Client::Process::serverIP;

int Client::Process::socket_ID;
struct sockaddr_in Client::Process::serverAddress;

pid_t Client::Process::processID;

/**
 * @brief Initializer of the Job Commander Process. Works like a constructor and initializes 
 * the appropriate data needed for communication with the server, the server name, the port 
 * number and the command of the user.
 * 
 * @param serverName the name of the server
 * @param portNum the port number of the server
 * @param command the command of the user
*/
void Client::Process::init(const std::string serverName, const port_num_t portNum, const std::string command) {
    
    Client::Process::serverName = serverName;
    Client::Process::portNum = portNum;
    Client::Process::command = command;

}

/**
 * @brief Receives the IP address of the server, according to its machine name that is
 * located to.
 * 
 * @return true if the IP was received successfully, false otherwise
*/
bool Client::Process::getServerIPAddress(void) {

    struct hostent* serverMachine;
    struct in_addr** addr_list;

    // Resolve the server machine name
    if ((serverMachine = gethostbyname(serverName.c_str())) == NULL) {
        std::cout << "Could not resolved name: " << Client::Process::serverName << std::endl;
        return false;
    }

    // Get the symbolic IP address of the server machine
    char symbolicIP[100];
    addr_list = (struct in_addr**)serverMachine->h_addr_list;
    for (unsigned int i = 0; addr_list[i] != NULL; i++) {
        strcpy(symbolicIP, inet_ntoa(*addr_list[i]));
    }

    Client::Process::serverIP = symbolicIP;

    return true;
}

/**
 * @brief Creates a new socket for the client in order to connect with the server and sets
 * the appropriate data for the server address.
 * 
 * @return true if the socket creation was successfull, false otherwise
*/
bool Client::Process::createSocket(void) {

    // Create socket
    if ((Client::Process::socket_ID = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating client socket");
        return false;
    }

    // Set up address data
    Client::Process::serverAddress.sin_family = AF_INET;
    Client::Process::serverAddress.sin_port = htons(Client::Process::portNum);
    inet_pton(AF_INET, serverIP.c_str(), &Client::Process::serverAddress.sin_addr.s_addr);

    return true;
}

/**
 * @brief Connects to the server with the corresponding name and port number.
 * 
 * @return true if the connection was successfull, false otherwise
*/
bool Client::Process::connectToServer(void) {
    
    // Connect to the server
    if (connect(Client::Process::socket_ID, (struct sockaddr*)&Client::Process::serverAddress, sizeof(Client::Process::serverAddress)) < 0) {
        perror("Connection to the server failed");
        return false;
    }

    return true;

}

/**
 * @brief Sends a specific user command to the server. Fist it sends the size of the command
 * to let the server know how many bytes to read, and then sends the actual command.
 * 
 * @return true if the command was sent successfully, false otherwise 
*/
bool Client::Process::sendCommand(void) {

    const char* message = Client::Process::command.c_str();
    ssize_t messageSize = strlen(message);

    send(Client::Process::socket_ID, &messageSize, sizeof(ssize_t), 0);
    send(Client::Process::socket_ID, message, messageSize, 0);

    return true;

}

/**
 * @brief Receives the response of the server after the command has been sent. It waits for a 
 * specific message to arrive and returns that message.
 * 
 * @return true if the respnse was received successfully, false otherwise
*/
bool Client::Process::receiveServerResponse(void) {

    CC::CC_Mode mode = getClientCommandMode(Client::Process::command);
    std::string serverResponse;

    if (mode == CC::JECC_ISSUE_JOB) {

        ClientCommunication::receiveIssueJobResponse(Client::Process::socket_ID, serverResponse);
        std::cout << serverResponse << std::endl;

        if (serverResponse != "JOB SUBMIT CANCELED BECAUSE OF SERVER TERMINATION") {
            ClientCommunication::receiveIssueJobResponse(Client::Process::socket_ID, serverResponse);
            std::cout << serverResponse << std::endl;
        }
    
    } else if (mode == CC::JECC_POLL) {

        ClientCommunication::receivePollResponse(Client::Process::socket_ID, serverResponse);
        std::cout << serverResponse << std::endl;

    } else if (mode == CC::JECC_STOP) {

        ClientCommunication::receiveStopResponse(Client::Process::socket_ID, serverResponse);
        std::cout << serverResponse << std::endl;

    } else if (mode == CC::JECC_SET_CONCURRENCY) {

        ClientCommunication::receiveSetConcurrencyResponse(Client::Process::socket_ID, serverResponse);
        std::cout << serverResponse << std::endl;

    } else if (mode == CC::JECC_EXIT) {

        ClientCommunication::receiveExitResponse(Client::Process::socket_ID, serverResponse);
        std::cout << serverResponse << std::endl;

    }

    return true;
    
}

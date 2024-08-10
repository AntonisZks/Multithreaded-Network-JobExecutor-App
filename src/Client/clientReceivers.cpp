/* Filename: clientReceivers.cpp */

#include <unistd.h>
#include "../../include/communication.h"
#include "../../include/clientCommands.h"

/* Namespace alias */
namespace ClientCommunication = Application_Client_Server_Communication::Application_Job_Commander_Client;
namespace CC = Application_Job_Commander_Client::Application_Client_Commands;

/**
 * @brief Handles receiving the server response, in case the client command to the server
 * was to issue a new job to the system. Then the corresponding response of the server
 * has to be a message that the job was submitted.
 * 
 * @param socketID the id of the socket used for communication
 * @param serverResponse the response of the server
 * 
 * @return true if the response was received successfully, false otherwise 
*/
bool ClientCommunication::receiveIssueJobResponse(const int socketID, std::string& serverResponse) {

    // Create variables that will hold the response and its size
    ssize_t responseSize;
    char* response;

    // First read the response size
    ssize_t bytesRead = read(socketID, &responseSize, sizeof(ssize_t));
    if (bytesRead == -1) {
        perror("Error receiving server response size");
        return false;
    } 
    else if (bytesRead != sizeof(ssize_t)) {
        std::cerr << "Incomplete read of response size" << std::endl;
        return false;
    }

    // Allocate memory for the response
    response = new char[responseSize + 1];
    if (response == nullptr) {
        perror("Error allocating memory");
        return false;
    }

    // Then read the actual response
    ssize_t totalBytesRead = 0;
    while (totalBytesRead < responseSize) 
    {
        ssize_t bytesRead = read(socketID, response + totalBytesRead, responseSize - totalBytesRead);
        
        if (bytesRead == -1) {
            perror("Error receiving server response");
            delete[] response;
            return false;
        }

        totalBytesRead += bytesRead;
    }

    // Initialize the server response
    response[responseSize] = '\0';
    serverResponse = response;

    delete[] response;

    return true;
}

/**
 * @brief Handles to receive the server response, in case the client command to the server
 * was to set the concurrency. Then the corresponding response of the server has to be a message
 * saying that the concurrency was set successully.
 * 
 * @param socketID the id of the socket used for communication
 * @param serverResponse the response of the server
 * 
 * @return true if the response was received successfully, false otherwise 
*/
bool ClientCommunication::receiveSetConcurrencyResponse(const int socketID, std::string& serverResponse) {

    ssize_t responseSize;
    char* response;

    read(socketID, &responseSize, sizeof(ssize_t));
 
    response = new char[responseSize + 1];
    if (response == nullptr) {
        perror("Error allocating memory");
        return false;
    }

    read(socketID, response, responseSize);

    serverResponse = response;

    delete[] response;

    return true;    

}

/**
 * @brief Handles to receive the server response, in case the client command to the server
 * was to poll. Then the corresponding response of the server has to be a message that contains
 * a list of all the job triplates that are waiting to be executed.
 * 
 * @param socketID the id of the socket used for communication
 * @param serverResponse the response of the server
 * 
 * @return true if the response was received successfully, false otherwise 
*/
bool ClientCommunication::receivePollResponse(const int socketID, std::string& serverResponse) {

    CC::JobTriplate triplate;
    ssize_t bufferItems;

    read(socketID, &bufferItems, sizeof(ssize_t));
    
    for (unsigned int i = 0; i < bufferItems; i++) {
        
        char* message;
        ssize_t messageSize;

        read(socketID, &messageSize, sizeof(ssize_t));
        
        message = new char[messageSize];
        if (message == nullptr) {
            perror("Error allocating memory");
            return false;
        }

        read(socketID, message, messageSize);

        std::string currentMessage = message;

        delete[] message;

        serverResponse += currentMessage;

        if (i < bufferItems - 1) {
            serverResponse += "\n";
        }       
    }

    return true;
}

/**
 * @brief Handles to receive the server response, in case the client command to the server
 * was to stop a specific job. Then the corresponding response of the server has to be a message
 * saying if the corresponding job ID was removed or not found in the buffer.
 * 
 * @param socketID the id of the socket used for communication
 * @param serverResponse the response of the server
 * 
 * @return true if the response was received successfully, false otherwise 
*/
bool ClientCommunication::receiveStopResponse(const int socketID, std::string& serverResponse) {

    ssize_t responseSize;
    char* response;

    read(socketID, &responseSize, sizeof(ssize_t));
 
    response = new char[responseSize + 1];
    if (response == nullptr) {
        perror("Error allocating memory");
        return false;
    }

    read(socketID, response, responseSize);

    serverResponse = response;

    delete[] response;

    return true;

}

/**
 * @brief Handles to receive the server response, in case the client command to the server
 * was to exit. Then the corresponding response of the server has to be a message
 * saying the server has been terminated.
 * 
 * @param socketID the id of the socket used for communication
 * @param serverResponse the response of the server
 * 
 * @return true if the response was received successfully, false otherwise 
*/
bool ClientCommunication::receiveExitResponse(const int socketID, std::string& serverResponse) {

    ssize_t responseSize;
    char* response;

    read(socketID, &responseSize, sizeof(ssize_t));
 
    response = new char[responseSize + 1];
    if (response == nullptr) {
        perror("Error allocating memory");
        return false;
    }

    read(socketID, response, responseSize);

    serverResponse = response;

    delete[] response;

    return true;

}

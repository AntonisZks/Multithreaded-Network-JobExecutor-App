/* Filename: communication.h */

#pragma once

#include <iostream>
#include <string>

namespace Application_Client_Server_Communication {

    namespace Application_Job_Commander_Client {
    
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
        bool receiveIssueJobResponse(const int socketID, std::string& serverResponse);

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
        bool receiveSetConcurrencyResponse(const int socketID, std::string& serverResponse);

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
        bool receivePollResponse(const int socketID, std::string& serverResponse);

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
        bool receiveStopResponse(const int socketID, std::string& serverResponse);

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
        bool receiveExitResponse(const int socketID, std::string& serverResponse);

    }

}

/* Filename: jobCommanderProcess.h */

#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

typedef unsigned int port_num_t;

namespace Application_Job_Commander_Client {

    /**
     * @brief Public class that represents a Job Commander Process. It contains all
     * the basic and appropriate data for communication with the server, and implements 
     * the necessary methods to send and receive data.
     * 
     * @author Antonis Zikas sdi21000388
    */
    class Process {
    private:

        static std::string serverName; // Server name 
        static std::string command;    // User command
        static port_num_t portNum;     // Port Number
        static std::string serverIP;   // Server IP Address

        static int socket_ID;                    // The ID of the socket for communication
        static struct sockaddr_in serverAddress; // The address of the server connected

        static pid_t processID; // Process ID

    public:

        /**
         * @brief Initializer of the Job Commander Process. Works like a constructor and initializes 
         * the appropriate data needed for communication with the server, the server name, the port 
         * number and the command of the user.
         * 
         * @param serverName the name of the server
         * @param portNum the port number of the server
         * @param command the command of the user
        */
        static void init(const std::string serverName, const port_num_t portNum, const std::string command);

        /**
         * @brief Receives the IP address of the server, according to its machine name that is
         * located to.
         * 
         * @return true if the IP was received successfully, false otherwise
        */
        static bool getServerIPAddress(void);

        /**
         * @brief Creates a new socket for the client in order to connect with the server and sets
         * the appropriate data for the server address.
         * 
         * @return true if the socket creation was successfull, false otherwise
        */
        static bool createSocket(void);

        /**
         * @brief Connects to the server with the corresponding name and port number.
         * 
         * @return true if the connection was successfull, false otherwise
        */
        static bool connectToServer(void);

        /**
         * @brief Sends a specific user command to the server. Fist it sends the size of the command
         * to let the server know how many bytes to read, and then sends the actual command.
         * 
         * @return true if the command was sent successfully, false otherwise 
        */
        static bool sendCommand(void);

        /**
         * @brief Receives the response of the server after the command has been sent. It waits for a 
         * specific message to arrive and returns that message.
         * 
         * @return true if the respnse was received successfully, false otherwise
        */
        static bool receiveServerResponse(void);

    };

}

namespace Client = Application_Job_Commander_Client; // namespace alias

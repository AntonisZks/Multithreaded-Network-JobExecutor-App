/* Filename: clientCommands.h */

#pragma once

#include <iostream>
#include <string>

namespace Application_Job_Commander_Client {

    namespace Application_Client_Commands {

        /**
         * @brief Enumeration that contains all the possible modes a specific command, sent from a
         * client to the server, can have.
         * 
         * @author Antonis Zikas sdi2100038
        */
        typedef enum Application_Client_Command_Mode {
            
            JECC_ISSUE_JOB,       // Stands for 'issueJob <job>' command
            JECC_SET_CONCURRENCY, // Stands for 'setConcurrency <N>' command
            JECC_STOP,            // Stands for 'stop <jobID>' command
            JECC_POLL,            // Stands for 'stop [running, queued]' command
            JECC_EXIT,            // Stands for 'exit' command, in order to terminate the server

            JECC_INVALID // Stands for invalid command mode
        
        } CC_Mode;

        /**
         * @brief Public struct that represents the client command job triplate, which holds the appropriate
         * data for a job. The jobID, the full job and the connection socket ID.
         * 
         * @author Antonis Zikas sdi2100038
        */
        typedef struct Application_Client_Command_Job_Triplate {

            std::string jobID; // The job ID of the client command job
            std::string job;   // The actual job of the client command
            int socketID;      // The socket ID on which the connection was occured

        } JobTriplate;

    }

}

/**
 * @brief Receives a specific client command as a string and returns its mode (ISSUE_JOB, POLL,
 * SET_CONCURRENCY, STOP or EXIT).
 * 
 * @param command the client command as a string
 * 
 * @return the mode of the given client command
*/
Application_Job_Commander_Client::Application_Client_Commands::CC_Mode getClientCommandMode(const std::string command);

/**
 * @brief Overloading operator << function that is being used to print a specific client command job
 * triplate to the tty.
 * 
 * @param out an output stream object
 * @param triplate the job triplate to print
 * 
 * @return the result output stream
*/
std::ostream& operator<<(std::ostream& out, const Application_Job_Commander_Client::Application_Client_Commands::JobTriplate triplate);

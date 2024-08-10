/* Filename: controllerThread.h */

#pragma once

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include "clientCommands.h"
#include "jobExecutorServerProcess.h"

extern bool serverShouldStop;

namespace Application_Job_Executor_Server {

    namespace Application_Controller_Thread {

        /**
         * @brief Public Class that represents the Controller Thread of the application.
         * It contains the socket ID of the server that created an instance of this class
         * and also the appropriate methods to build the basic algorithm of this thread.
         * 
         * @author Antonis Zikas sdi2100038
        */
        class Thread {
        
        private:

            int clientSocket;              // The socket of the server
            std::string clientCommand;     // The command that has been sent from a client
            CC::CC_Mode clientCommandMode; // The mode of the client command

            static unsigned int jobsEntered;

            /**
             * @brief Handles the issueJob client command. It receives the full command of the
             * client, determines what the bash job is and puts it to the common queue buffer
             * to be executed later on by a worker thread.
             * 
             * @return true, if the process was successfull, false otherwise
            */
            bool insertNewJobToBufferQueue(void);

            /**
             * @brief Handles the setConcurrency client command. It determines what the given 
             * concurrency is and sets it as the new one in the application. 
             * 
             * @return true, if the process was successfull, false otherwise
            */
            bool setServerConcurrencyLevel(void);

            /**
             * @brief Handles the poll client command. It iterates through the waiting buffer
             * queue, and sends each individual job triplate back to the client.
             * 
             * @return true, if the process was successfull, false otherwise
            */
            bool sendWaitingJobsToClient(void);

            /**
             * @brief Handles the stop client command. It itrates through the waiting buffer
             * queue, until it finds the job specified to be removed from the buffer, according
             * to its job ID.
             * 
             * @return true, if the process was successfull, false otherwise
            */
            bool removeJobFromBufferQueue(void);

            /**
             * @brief Handles the exit client command. It sends a termination message back to the
             * client and terminates the server.
             * 
             * @return true, if the process was successfull, false otherwise
            */
            bool terminateServer(void);

        public:

            static bool shouldStop;

            /**
             * @brief Constructor of the Controller Thread. It stores the socket of the client
             * that is being used for communication with the client.
             * 
             * @param clientSocket the socket id of the client
            */
            Thread(const int clientSocket);

            /**
             * @brief Receives the command that a client has sent to the server and returns
             * that command as a C++ string.
             * 
             * @return true if the command was received successfully, false otherwise
            */
            bool receiveClientCommandFromSocket(void);

            /**
             * @brief Executes the appropriate task of the controller thread, according to
             * the client command received and its mode.
             * 
             * @return true if the task was done successfully, false otherwise 
            */
            bool executeTask(void);
        };

    }

}

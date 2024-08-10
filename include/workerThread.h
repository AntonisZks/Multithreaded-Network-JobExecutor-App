/* Filename: workerThread.h */

#pragma once

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "clientCommands.h"
#include "jobExecutorServerProcess.h"

namespace Application_Job_Executor_Server {

    namespace Application_Worker_Thread {

        /**
         * @brief Public class that represents the Worker Thread of the application. It
         * contains the socket ID of the server that created an instance of this class
         * and also the appropriate methods to build the basic algorithm of this thread.
         * 
         * @author Antonis Zikas
        */
        class Thread {
        
        private:

            int clientSocket; 
            pid_t childProcessID;       

            /**
             * @brief Sends the output of the job executed by the thread back to the client.
             * 
             * @param output the output of the job
             * @param outputSize the size of the output
             * 
             * @return true if the output was sent successfully, false otherwise
            */
            bool sendJobOutputToClient(const char* output, const ssize_t outputSize);

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
            char* createOutputResponse(const char* jobOutput, const ssize_t jobOutputSize, 
                const char* jobID, ssize_t& responseSize
            );

        public:

            /**
             * @brief Receives and returns a job from the waiting buffer queue, in order to
             * be executed. It also initializes the client socket ID from the triplate.
             * 
             * @return the first job triplate of the waiting buffer queue
            */
            CC::JobTriplate receiveJobFromBuffer(void);

            /**
             * @brief Receives a job triplate and executes its corresponding job. It creates 
             * a child process with fork() system call and uses exec() to execute the given
             * process
             * 
             * @param jobTriplate the triplate containing the job to execute
             * 
             * @return true if the job was executed successfully, false otherwise 
            */
            bool executeJob(const CC::JobTriplate jobTriplate);


        };

    }

}

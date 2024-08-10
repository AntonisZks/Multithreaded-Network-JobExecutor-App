/* Filename: waitingBufferQueue.h */

#pragma once

#include <iostream>
#include <vector>
#include "clientCommands.h"

/* Namespace Alias */
namespace CC = Application_Job_Commander_Client::Application_Client_Commands;


namespace Application_Job_Executor_Server {

    namespace Application_Common_Waiting_Buffer {

        /**
         * @brief Public Static class that represents the common waiting buffer queue
         * for the job triplates received from client commands. These job triplates 
         * will placed inside this structure by a Controller Thread.
         * 
         * @author Antonis Zikas sdi2100038
        */
        class Queue {
        
        private:

            static size_t capacity;                     // The maximum size of the buffer queue
            static size_t size;                         // The current size of the buffer queue
            static std::vector<CC::JobTriplate> buffer; // The queue structure

        public:

            /**
             * @brief Returns the maximum size of the waiting buffer queue.
             * 
             * @return the capacity of the queue.
            */
            static size_t getCapacity(void);

            /**
             * @brief Returns the current size of the waiting buffer queue.
             * 
             * @return the size of the queue.
            */
            static size_t getSize(void);

            /**
             * @brief Sets the capacity of the waiting buffer queue.
             * 
             * @param capacity the capacity to be set
            */
            static void setCapacity(const size_t capacity);

            /**
             * @brief Inserts a new client command job triplate to the very end of the 
             * waiting buffer queue.
             * 
             * @param triplate the triplate to insert
             * 
             * @return true if the insertion was successfull, false otherwise
            */
            static void insertJobTriplate(const CC::JobTriplate triplate);

            /**
             * @brief Removes and returns the job triplate located at the begining of the 
             * waiting buffer queue.
             * 
             * @return a pointer to the job triplate at the beggining of the queue, NULL 
             * if the queue is empty
            */
            static CC::JobTriplate getJobTriplate(void);

            /**
             * @brief Searches for the job triplate with the specific job ID and if it is
             * found, it removes it from the waiting buffer queue.
             * 
             * @param job_ID the job ID to be removed
             * @param jobTriplate the triplate that has been removed
             * 
             * @return true if the job ID was found, false otherwise
            */
            static bool removeJobTriplateByID(const std::string job_ID, CC::JobTriplate& jobTriplate);

            /**
             * @brief Returns the corresponding job triplate located at the specific given 
             * index inside the waiting buffer queue structure.
             * 
             * @param index the index of the requested triplate
             * 
             * @return the corresponding triplate at the given index
            */
            static CC::JobTriplate at(const unsigned int index);

            /**
             * @brief Returns whether the waiting buffer queue is full or not.
             * 
             * @return true if the queue is full, false otherwise
            */
            static bool isFull(void);

            /**
             * @brief Returns whether the waiting buffer queue is empty or not.
             * 
             * @return true if the queue is empty, false otherwise
            */
            static bool isEmpty(void);

        };

    }

}

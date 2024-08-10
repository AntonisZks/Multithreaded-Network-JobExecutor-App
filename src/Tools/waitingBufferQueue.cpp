/* Filename waitingBufferQueue.cpp */

#include "../../include/waitingBufferQueue.h"

namespace WaitingBuffer = Application_Job_Executor_Server::Application_Common_Waiting_Buffer; // namespace alias

// Initialize the static members
size_t WaitingBuffer::Queue::capacity;
size_t WaitingBuffer::Queue::size;
std::vector<CC::JobTriplate> WaitingBuffer::Queue::buffer;

/**
 * @brief Returns the maximum size of the waiting buffer queue.
 * 
 * @return the capacity of the queue.
*/
size_t WaitingBuffer::Queue::getCapacity(void) {

    return WaitingBuffer::Queue::capacity;

}

/**
 * @brief Returns the current size of the waiting buffer queue.
 * 
 * @return the size of the queue.
*/
size_t WaitingBuffer::Queue::getSize(void) {

    return WaitingBuffer::Queue::size;

}

/**
 * @brief Sets the capacity of the waiting buffer queue.
 * 
 * @param capacity the capacity to be set
*/
void WaitingBuffer::Queue::setCapacity(const size_t capacity) {

    WaitingBuffer::Queue::capacity = capacity;

}

/**
 * @brief Inserts a new client command job triplate to the very end of the 
 * waiting buffer queue.
 * 
 * @param triplate the triplate to insert
 * 
 * @return true if the insertion was successfull, false otherwise
*/
void WaitingBuffer::Queue::insertJobTriplate(const CC::JobTriplate triplate) {

    // Try to insert the triplate and check if the buffer is full
    if (WaitingBuffer::Queue::size < WaitingBuffer::Queue::capacity) {

        WaitingBuffer::Queue::buffer.push_back(triplate);
        WaitingBuffer::Queue::size++;
    
    } else {
        std::cerr << "Cannot insert job triplate. Waiting Buffer is full." << std::endl;
    }

}

/**
 * @brief Removes and returns the job triplate located at the begining of the 
 * waiting buffer queue.
 * 
 * @return the job triplate at the beggining of the queue
*/
CC::JobTriplate WaitingBuffer::Queue::getJobTriplate(void) {

    // Otherwise return the first item and remove it
    CC::JobTriplate triplate = WaitingBuffer::Queue::buffer.front();
    WaitingBuffer::Queue::buffer.erase(WaitingBuffer::Queue::buffer.begin());
    WaitingBuffer::Queue::size--;

    return triplate;
}

/**
 * @brief Searches for the job triplate with the specific job ID and if it is
 * found, it removes it from the waiting buffer queue.
 * 
 * @param job_ID the job ID to be removed
 * @param jobTriplate the triplate that has been removed
 * 
 * @return true if the job ID was found, false otherwise
*/
bool WaitingBuffer::Queue::removeJobTriplateByID(const std::string job_ID, CC::JobTriplate& jobTriplate) {

    // Search for the given job ID in the buffer
    for (unsigned int i = 0; i < WaitingBuffer::Queue::size; i++) {

        CC::JobTriplate triplate = WaitingBuffer::Queue::at(i);

        // If it is found remove it
        if (triplate.jobID == job_ID) {

            WaitingBuffer::Queue::buffer.erase(WaitingBuffer::Queue::buffer.begin() + i);
            WaitingBuffer::Queue::size--;
            jobTriplate = triplate;
            return true;

        }

    }

    return false;

}

/**
 * @brief Returns the corresponding job triplate located at the specific given 
 * index inside the waiting buffer queue structure.
 * 
 * @param index the index of the requested triplate
 * 
 * @return the corresponding triplate at the given index
*/
CC::JobTriplate WaitingBuffer::Queue::at(const unsigned int index) {

    return WaitingBuffer::Queue::buffer.at(index);

}

/**
 * @brief Returns whether the waiting buffer queue is full or not.
 * 
 * @return true if the queue is full, false otherwise
*/
bool WaitingBuffer::Queue::isFull(void) {

    return WaitingBuffer::Queue::size == WaitingBuffer::Queue::capacity;

}

/**
 * @brief Returns whether the waiting buffer queue is empty or not.
 * 
 * @return true if the queue is empty, false otherwise
*/
bool WaitingBuffer::Queue::isEmpty(void) {

    return WaitingBuffer::Queue::size == 0;

}

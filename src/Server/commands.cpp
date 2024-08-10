/* Filename: commands.cpp */

#include "../../include/clientCommands.h"
#include "../../include/common.h"

/* Namespace Alias */
namespace CC = Application_Job_Commander_Client::Application_Client_Commands;

/**
 * @brief Receives a specific client command as a string and returns its mode (ISSUE_JOB, POLL,
 * SET_CONCURRENCY, STOP or EXIT).
 * 
 * @param command the client command as a string
 * 
 * @return the mode of the given client command
*/
CC::CC_Mode getClientCommandMode(const std::string command) {

    CC::CC_Mode commandMode;
    std::string firstArgument = getFirstWord(command);
    
    // Determin the job type and return it
    if (firstArgument == "issueJob") { commandMode = CC::JECC_ISSUE_JOB; }
    else if (firstArgument == "setConcurrency") { commandMode = CC::JECC_SET_CONCURRENCY; }
    else if (firstArgument == "poll") { commandMode = CC::JECC_POLL; }
    else if (firstArgument == "stop") { commandMode = CC::JECC_STOP; }
    else if (firstArgument == "exit") { commandMode = CC::JECC_EXIT; }
    else { commandMode = CC::JECC_INVALID; }

    return commandMode;

}

/**
 * @brief Overloading operator << function that is being used to print a specific client command job
 * triplate to the tty.
 * 
 * @param out an output stream object
 * @param triplate the job triplate to print
 * 
 * @return the result output stream
*/
std::ostream& operator<<(std::ostream& out, const CC::JobTriplate triplate) {

    out << "(" << triplate.jobID << ", " << triplate.job << ", " << triplate.socketID << ")";
    return out;

}

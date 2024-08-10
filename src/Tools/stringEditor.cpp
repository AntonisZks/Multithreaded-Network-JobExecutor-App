/* Filename: stringEditor.cpp */

#include "../../include/common.h"

/**
 * @brief Supporting function that returns the first word of a given string,
 * according to the given seperator value.
 * 
 * @param command the given string
 * 
 * @return the first word of the string
*/
std::string getFirstWord(const std::string str, const char seperator) {

    size_t pos = str.find(seperator); // Find the first seperator character in the command

    // If no space is found then the entire command is a single word
    if (pos == std::string::npos) {
        return str;
    }

    // Otherwise return the substring of the command from the start up to the first space
    return str.substr(0, pos);
}

/**
 * @brief Supporting function that receives a string and returns the same string
 * except for the first word, according to the seperator
 * 
 * @param str the string to edit
 * 
 * @return the string without the first word.
*/
std::string removeFirstWord(const std::string str, const char seperator) {
    
    size_t pos = str.find(seperator); // Find the first seperator character in the command

    // If no space found return an empty string
    if (pos == std::string::npos) {
        return "";
    }

    // Otherwise return the substring starting from the second word up to the end
    return str.substr(pos + 1);

}

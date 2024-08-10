/* Filename: common.h */

#pragma once

#include <iostream>
#include <string>

/**
 * @brief Supporting function that returns the first word of a given string,
 * according to the given seperator value.
 * 
 * @param command the given string
 * 
 * @return the first word of the string
*/
std::string getFirstWord(const std::string str, const char seperator=' ');

/**
 * @brief Supporting function that receives a string and returns the same string
 * except for the first word, according to the seperator
 * 
 * @param str the string to edit
 * 
 * @return the string without the first word.
*/
std::string removeFirstWord(const std::string str, const char seperator=' ');

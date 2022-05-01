#ifndef __OPTIONS_HEADER_FILE__
#define __OPTIONS_HEADER_FILE__

#include <string>
#include <algorithm>

char* getCmdOption(char ** begin, char ** end, const std::string & option);
bool cmdOptionExists(char** begin, char** end, const std::string& option);

#endif // __OPTIONS_HEADER_FILE__


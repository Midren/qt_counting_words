#ifndef PARALLEL_READING_UTILS_H
#define PARALLEL_READING_UTILS_H

#include <vector>
#include <string>
#include <fstream>
#include "zip.h"
#include <map>
#include <sstream>
#include <boost/algorithm/string.hpp>

std::string check_input(const std::string &file_name);

std::string get_file_ext(std::string file_name);

void unzip_files(std::string dst, std::string src);

#endif //PARALLEL_READING_UTILS_H

//
// Created by YuraYeliseev on 2019-04-23.
//

#ifndef PARALLEL_READING_CONFIG_PARSER_H
#define PARALLEL_READING_CONFIG_PARSER_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

struct Attributes {
    std::string infile;
    std::string out_by_a;
    std::string out_by_n;
    std::string NThreads;
};

Attributes *get_intArgs(const std::string &filename);

#endif //PARALLEL_READING_CONFIG_PARSER_H

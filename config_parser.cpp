//
// Created by YuraYeliseev on 2019-04-23.
//

#include "config_parser.h"
#include <iostream>

Attributes *get_intArgs(const std::string &filename) {
/*You need to pass parameters in such way
 *  infile;
    out_by_a;
    out_by_n;
    threads;
*/
    auto *attributes = new Attributes;
    std::map<std::string, std::string *> conf_attributes{{"in_dir",   &attributes->infile},
                                                         {"out_by_a", &attributes->out_by_a},
                                                         {"out_by_n", &attributes->out_by_n},
                                                         {"threads",  &attributes->NThreads}};
    std::ifstream f(filename);
    std::string line;
    while (getline(f, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        if (line[0] == '#' || line.empty()) {
            continue;
        }
        size_t half = line.find('=');
        std::string name = line.substr(0, half);
        std::string value = line.substr(half + 1);
        if (!std::any_of(conf_attributes.begin(), conf_attributes.end(), [name](auto x) { return x.first == name; })) {
            throw std::invalid_argument("Invalid config argument!");
        }
        try {
            *conf_attributes[name] = value;
        }
        catch (int e) {
            throw std::invalid_argument("Invalid config argument!");
        }
    }
    f.close();
    return attributes;
}
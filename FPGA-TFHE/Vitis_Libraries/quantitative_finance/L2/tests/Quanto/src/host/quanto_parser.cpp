/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "quanto_host.hpp"

static bool validate_parameters(struct parsed_params* p) {
    bool ret = false;
    if (p->validation == 0x3FF) {
        ret = true;
    }
    return ret;
}

static void init_validation(struct parsed_params* p) {
    p->validation = 0;
}

static bool process_parameter(struct parsed_params* p, std::string parm, std::string value) {
    try {
        if (parm.compare("s") == 0) {
            p->s = std::stod(value);
            p->validation |= 0x001;
        } else if (parm.compare("k") == 0) {
            p->k = std::stod(value);
            p->validation |= 0x002;
        } else if (parm.compare("v") == 0) {
            p->v = std::stod(value);
            p->validation |= 0x004;
        } else if (parm.compare("t") == 0) {
            p->t = std::stod(value);
            p->validation |= 0x008;
        } else if (parm.compare("r_domestic") == 0) {
            p->rd = std::stod(value);
            p->validation |= 0x010;
        } else if (parm.compare("r_foreign") == 0) {
            p->rf = std::stod(value);
            p->validation |= 0x020;
        } else if (parm.compare("q") == 0) {
            p->q = std::stod(value);
            p->validation |= 0x040;
        } else if (parm.compare("E") == 0) {
            p->E = std::stod(value);
            p->validation |= 0x080;
        } else if (parm.compare("fxv") == 0) {
            p->fxv = std::stod(value);
            p->validation |= 0x100;
        } else if (parm.compare("corr") == 0) {
            p->corr = std::stod(value);
            p->validation |= 0x200;
        } else if (parm.compare("exp") == 0) { // optional field
            p->exp = std::stod(value);
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

static std::string trim(std::string s) {
    const char* t = " \t\n\r\f\v";
    s.erase(0, s.find_first_not_of(t));
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

static bool process_token(struct parsed_params* p, std::string token) {
    std::istringstream iss_token(token);
    std::string parm;
    if (!std::getline(iss_token, parm, '=')) {
        return false;
    }

    std::string value;
    if (!std::getline(iss_token, value, '=')) {
        return false;
    }

    std::string dummy;
    if (std::getline(iss_token, dummy, '=')) {
        return false;
    }
    parm = trim(parm);
    value = trim(value);
    return process_parameter(p, parm, value);
}

bool is_comment(std::string line) {
    bool ret = false;
    std::size_t found = line.find_first_not_of(" \n\r\t");
    if (line[found] == '#') {
        ret = true;
    }
    return ret;
}

static struct parsed_params* process_line(std::string line, int line_num) {
    std::istringstream iss_line(line);
    std::string token;

    struct parsed_params* p = new struct parsed_params;

    init_validation(p);
    while (std::getline(iss_line, token, ',')) {
        if (!process_token(p, token)) {
            // cleanup
            delete p;
            std::cout << "ERROR: line(" << line_num << "): failed to parse token (" << token << ")" << std::endl;
            return nullptr;
        }
    }
    if (!validate_parameters(p)) {
        // cleanup
        delete p;
        std::cout << "ERROR: line(" << line_num << "): " << line << ": invalid parameters: (" << p->validation << ")"
                  << std::endl;
        return nullptr;
    }
    return p;
}

void cleanup_parameters(std::vector<struct parsed_params*>* vect) {
    for (auto x : *vect) {
        delete x;
    }
    delete vect;
}

std::vector<struct parsed_params*>* parse_file(std::string file) {
    std::ifstream ifs(file, std::ifstream::in);
    if (!ifs.is_open()) {
        std::cout << "ERROR: Failed to open file:" << file << std::endl;
        return nullptr;
    }

    std::vector<struct parsed_params*>* vect = new std::vector<struct parsed_params*>();
    struct parsed_params* p;

    std::string line;
    int line_number = 0;
    while (std::getline(ifs, line)) {
        line_number++;
        if (is_comment(line)) {
            continue;
        }

        if (line.empty()) {
            continue;
        }

        if ((p = process_line(line, line_number)) == nullptr) {
            // cleanup
            cleanup_parameters(vect);
            return nullptr;
        }
        vect->push_back(p);
    }
    return vect;
}

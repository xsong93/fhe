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

#ifndef X_UTILS_HPP
#define X_UTILS_HPP

#include <sys/time.h>

#include <algorithm>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

namespace x_utils {

inline int tvdiff(const timeval& tv0, const timeval& tv1) {
    return (tv1.tv_sec - tv0.tv_sec) * 1000000 + (tv1.tv_usec - tv0.tv_usec);
}

inline int tvdiff(const timeval& tv0, const timeval& tv1, const char* info) {
    int exec_us = tvdiff(tv0, tv1);
    printf("%s: %d.%03d msec\n", info, (exec_us / 1000), (exec_us % 1000));
    return exec_us;
}

class ArgParser {
   public:
    ArgParser(int argc, const char* argv[]) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
    }
    bool getCmdOption(const std::string option, std::string& value) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end()) {
            if (++itr != this->mTokens.end()) {
                value = *itr;
            }
            return true;
        }
        return false;
    }

   private:
    std::vector<std::string> mTokens;
};

inline bool has_end(std::string const& full, std::string const& end) {
    if (full.length() >= end.length()) {
        return (0 == full.compare(full.length() - end.length(), end.length(), end));
    } else {
        return false;
    }
}

inline bool is_dir(const char* path) {
    struct stat info;
    if (stat(path, &info) != 0) return false;
    if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

inline bool is_dir(const std::string& path) {
    return is_dir(path.c_str());
}

inline bool is_file(const char* path) {
    struct stat info;
    if (stat(path, &info) != 0) return false;
    if (info.st_mode & (S_IFREG | S_IFLNK))
        return true;
    else
        return false;
}

inline bool is_file(const std::string& path) {
    return is_file(path.c_str());
}

} // namespace x_utils

#endif // X_UTILS_HPP

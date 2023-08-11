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

#include "test.hpp"

#include <hls_stream.h>
#include <ap_int.h>

#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
using namespace std;
#include <sstream>
#include <string>
#include <vector>

// number of times to perform the test in different message and length
#define NUM_TESTS 1000
// the size of each message word in byte
#define MSG_SIZE 8
// the size of the digest in byte
#define DIG_SIZE 32

// table to save each message and its hash value
struct Test {
    string msg;
    unsigned char hash[DIG_SIZE];
    Test(const char* m, const void* h) : msg(m) { memcpy(hash, h, DIG_SIZE); }
};

// print hash value
std::string hash2str(unsigned char* h, int len) {
    ostringstream oss;
    string retstr;

    // check output
    oss.str("");
    oss << hex;
    for (int i = 0; i < len; i++) {
        oss << setw(2) << setfill('0') << (unsigned)h[i];
    }
    retstr = oss.str();
    return retstr;
}

int main() {
    std::cout << "*************************************" << std::endl;
    std::cout << "   Testing SHA3-256 on HLS project   " << std::endl;
    std::cout << "*************************************" << std::endl;

    // the original message to be digested
    const char message[] =
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz"
        "abcdefghijklmnopqrstuvwxyz";
    vector<Test> tests;

    // open file
    FILE* fp = fopen("testcases.dat", "r");

    // generate golden
    for (unsigned int i = 0; i < NUM_TESTS; i++) {
        unsigned int len = i % 256;
        char m[256];
        if (len != 0) {
            memcpy(m, message, len);
        }
        m[len] = 0;
        unsigned char h[DIG_SIZE];

        // read golden hash values from testcases.dat
        fread(h, sizeof(unsigned char), DIG_SIZE, fp);

        tests.push_back(Test(m, h));
    }
    // close file
    fclose(fp);

    unsigned int nerror = 0;
    unsigned int ncorrect = 0;

    hls::stream<ap_uint<8 * MSG_SIZE> > msg_strm("msg_strm");
    hls::stream<ap_uint<128> > len_strm("len_strm");
    hls::stream<bool> end_len_strm("end_len_strm");
    hls::stream<ap_uint<8 * DIG_SIZE> > digest_strm("digest_strm");
    hls::stream<bool> end_digest_strm("end_digest_strm");

    // generate input message words
    for (vector<Test>::const_iterator test = tests.begin(); test != tests.end(); test++) {
        ap_uint<8 * MSG_SIZE> msg;
        unsigned int n = 0;
        unsigned int cnt = 0;
        // write msg stream word by word
        for (string::size_type i = 0; i < (*test).msg.length(); i++) {
            if (n == 0) {
                msg = 0;
            }
            msg.range(7 + 8 * n, 8 * n) = (unsigned)((*test).msg[i]);
            n++;
            if (n == MSG_SIZE) {
                msg_strm.write(msg);
                ++cnt;
                n = 0;
            }
        }
        // deal with the condition that we didn't hit a boundary of the last word
        if (n != 0) {
            msg_strm.write(msg);
            ++cnt;
        }
        // inform the prmitive how many bytes do we have in this message
        len_strm.write((unsigned long long)((*test).msg.length()));
        end_len_strm.write(false);
    }
    end_len_strm.write(true);

    // call fpga module
    test(msg_strm, len_strm, end_len_strm, digest_strm, end_digest_strm);

    // check result
    for (vector<Test>::const_iterator test = tests.begin(); test != tests.end(); test++) {
        ap_uint<8 * DIG_SIZE> digest = digest_strm.read();
        bool x = end_digest_strm.read();

        unsigned char hash[DIG_SIZE];
        for (unsigned int i = 0; i < DIG_SIZE; i++) {
            hash[i] = (unsigned char)(digest.range(7 + 8 * i, 8 * i).to_int() & 0xff);
        }

        if (memcmp((*test).hash, hash, DIG_SIZE)) {
            ++nerror;
            cout << "fpga   : " << hash2str((unsigned char*)hash, DIG_SIZE) << endl;
            cout << "golden : " << hash2str((unsigned char*)(*test).hash, DIG_SIZE) << endl;
        } else {
            ++ncorrect;
        }
    }

    bool x = end_digest_strm.read();

    if (nerror) {
        cout << "FAIL: " << dec << nerror << " errors found." << endl;
    } else {
        cout << "PASS: " << dec << ncorrect << " inputs verified, no error found." << endl;
    }

    return nerror;
}

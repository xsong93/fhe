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

#include <vector> // std::vector
#include <iostream>
#include <stdlib.h>

#include "hls_stream.h"
#include "xf_database/static_eval.hpp"
#define LEN 10 // number of test data generated

int user_func(int a, int b, int c, int d) {
    int res = (a >= 0 || b > 0) ? c * (1 - d) : 0;
    return res;
}

void dut(hls::stream<int>& in1_strm,
         hls::stream<int>& in2_strm,
         hls::stream<int>& in3_strm,
         hls::stream<int>& in4_strm,
         hls::stream<bool>& e_in_strm,
         hls::stream<int>& out_strm,
         hls::stream<bool>& e_out_strm) {
    xf::database::staticEval<int, int, int, int, int, user_func>(in1_strm, in2_strm, in3_strm, in4_strm, e_in_strm,
                                                                 out_strm, e_out_strm);
}

// generate a random integer sequence between speified limits a and b (a<b);
int rand_int(int a, int b) {
    return rand() % (b - a + 1) + a;
}

void generate_test_data(uint64_t len,
                        std::vector<int>& testvector1,
                        std::vector<int>& testvector2,
                        std::vector<int>& testvector3,
                        std::vector<int>& testvector4) {
    for (int i = 0; i < len; i++) {
        int randnum = rand_int(-100, 100);
        int randnum2 = rand_int(-100, 100);
        int randnum3 = rand_int(-100, 100);
        int randnum4 = rand_int(-100, 100);
        testvector1.push_back(randnum);
        testvector2.push_back(randnum2);
        testvector3.push_back(randnum3);
        testvector4.push_back(randnum4);
    }
}

int main() {
    hls::stream<int> in1_strm("in1_strm");
    hls::stream<int> in2_strm("in2_strm");
    hls::stream<int> in3_strm("in3_strm");
    hls::stream<int> in4_strm("in4_strm");
    hls::stream<bool> e_in_strm("e_in_strm");
    hls::stream<int> out_strm("out_strm");
    hls::stream<bool> e_out_strm("e_out_strm");

    // generate test dat
    std::vector<int> testvector1, testvector2, testvector3, testvector4;
    generate_test_data(LEN, testvector1, testvector2, testvector3, testvector4);
    // prepare data, write data into stream
    for (std::string::size_type i = 0; i < LEN; i++) {
        std::cout << "testvector data is " << testvector1[i] << ", " << testvector2[i] << "," << testvector3[i] << ","
                  << testvector4[i] << std::endl;
        in1_strm.write(testvector1[i]);
        in2_strm.write(testvector2[i]);
        in3_strm.write(testvector3[i]);
        in4_strm.write(testvector4[i]);
        e_in_strm.write(0);
    }
    e_in_strm.write(1);
    std::cout << std::endl;

    // call ALU_block
    dut(in1_strm, in2_strm, in3_strm, in4_strm, e_in_strm, out_strm, e_out_strm);

    int nerror = 0;
    //===== check if the output flag e_out_strm is correct or not =====
    for (int i = 0; i < LEN; i++) {
        bool e = e_out_strm.read();
        if (e) {
            nerror++;
            std::cout << "the alu flag is incorrect" << std::endl;
        }
    }
    // read out the last flag that e should =1
    bool e = e_out_strm.read();
    if (!e) {
        nerror++;
        std::cout << "the alu data is incorrect" << std::endl;
    }
    // =======check the result with referenece
    for (int i = 0; i < LEN; i++) {
        bool cmp_dout =
            (user_func(testvector1[i], testvector2[i], testvector3[i], testvector4[i]) == out_strm.read()) ? 1 : 0;
        if (!cmp_dout) {
            nerror++;
            std::cout << "the alu data is incorrect" << std::endl;
        }
    }

    if (nerror) {
        std::cout << "\nFAIL: nerror= " << nerror << " errors found.\n";
    } else {
        std::cout << "\nPASS: no error found.\n";
    }
    return nerror;
}

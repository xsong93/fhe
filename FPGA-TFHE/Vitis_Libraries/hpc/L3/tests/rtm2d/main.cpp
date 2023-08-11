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

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "binFiles.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "rtm2d_kernel.hpp"

using namespace std;

typedef WideData<RTM_dataType, RTM_nPE> RTM_wideType;
typedef WideData<RTM_wideType, 2> RTM_pairType;

int main(int argc, char** argv) {
    int err0 = 0, err1 = 0, err2 = 0, err3 = 0, err4 = 0, err5 = 0, err6 = 0, err7 = 0;
    bool pass0, pass1, pass2, pass3, pass4, pass5, pass6, pass7;

    const int requiredArg = 7;
    if (argc < requiredArg) return EXIT_FAILURE;

    unsigned int argId = 0;

    string l_xclbinFile = argv[++argId];
    string filePath = argv[++argId];
    int l_height = atoi(argv[++argId]);
    int l_width = atoi(argv[++argId]);
    int l_time = atoi(argv[++argId]);
    int l_shot = atoi(argv[++argId]);

    int l_imgX = l_width - 2 * RTM_NXB;
    int l_imgZ = l_height - 2 * RTM_NZB;
    int l_area = l_width * l_height / RTM_nPE;

    bool l_verify = false;
    if (argc > 1 + argId) l_verify = atoi(argv[++argId]) == 0 ? false : true;

    unsigned int l_deviceId = 0;
    if (argc > 1 + argId) l_deviceId = atoi(argv[++argId]);

    host_buffer_t<RTM_pairType> p_snap, p_p, p_r;
    host_buffer_t<RTM_dataType> p_upb, p_i;
    vector<RTM_dataType> p, pp, bp, bpp, pr, rr, snap0, snap1, pref, ppref, rref, ref, imlocref, p_img;

    p.resize(l_width * l_height);
    pp.resize(l_width * l_height);
    bp.resize(l_width * l_height);
    bpp.resize(l_width * l_height);
    rr.resize(l_width * l_height);
    pr.resize(l_width * l_height);

    FPGA fpga(l_xclbinFile.c_str(), l_deviceId);
    ForwardKernel<RTM_dataType, RTM_order, RTM_nPE> fwd(&fpga, l_height, l_width, RTM_NZB, RTM_NXB, l_time, l_shot);
    BackwardKernel<RTM_dataType, RTM_order, RTM_nPE> bwd(&fpga, l_height, l_width, RTM_NZB, RTM_NXB, l_time, l_shot);

    fwd.loadData(filePath);
    bwd.loadData(filePath);
    double elapsedF = 0, elapsedB = 0;

    auto start = chrono::high_resolution_clock::now();

    for (int s = 0; s < l_shot; s++) {
        elapsedF += fwd.run(s, l_width / 2, p_snap, p_upb);
        elapsedB += bwd.run(s, p_snap, p_upb, p_p, p_r, p_i);

        if (l_verify) {
            for (int i = 0; i < l_area; i++) {
                for (int pe = 0; pe < RTM_nPE; pe++) {
                    p[i * RTM_nPE + pe] = p_snap[i][1][pe];
                    pp[i * RTM_nPE + pe] = p_snap[i][0][pe];

                    bpp[i * RTM_nPE + pe] = p_p[i][0][pe];
                    bp[i * RTM_nPE + pe] = p_p[i][1][pe];
                    rr[i * RTM_nPE + pe] = p_r[i][0][pe];
                    pr[i * RTM_nPE + pe] = p_r[i][1][pe];
                }
            }

            readBin(filePath + "p0_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, ppref);
            readBin(filePath + "p1_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, pref);

            readBin(filePath + "r0_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, rref);
            readBin(filePath + "r1_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, ref);

            readBin(filePath + "imloc_s" + to_string(s) + ".bin", sizeof(float) * l_imgX * l_imgZ, imlocref);

            pass1 = compare<RTM_dataType>(l_width * l_height, bp.data(), pref.data(), err1);
            cout << "There are in total " << err1 << " errors in bp v.s. pref" << endl;

            pass2 = compare<RTM_dataType>(l_width * l_height, bpp.data(), ppref.data(), err2);
            cout << "There are in total " << err2 << " errors in bpp v.s. ppref" << endl;

            pass3 = compare<RTM_dataType>(l_width * l_height, pr.data(), ref.data(), err3);
            cout << "There are in total " << err3 << " errors in pr v.s. prref" << endl;

            pass4 = compare<RTM_dataType>(l_width * l_height, rr.data(), rref.data(), err4);
            cout << "There are in total " << err4 << " errors in rr v.s. pprref" << endl;

            readBin(filePath + "snap0_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, snap0);
            readBin(filePath + "snap1_s" + to_string(s) + ".bin", sizeof(float) * l_width * l_height, snap1);

            pass5 = compare<RTM_dataType>(l_width * l_height, pp.data(), snap0.data(), err5);
            cout << "There are in total " << err5 << " errors in pp v.s. snap0" << endl;

            pass6 = compare<RTM_dataType>(l_width * l_height, p.data(), snap1.data(), err6);
            cout << "There are in total " << err6 << " errors in p v.s. snap1" << endl;

            if (pass1 && pass2 && pass3 && pass4 && pass5 && pass6) {
                cout << "Test passed!" << endl;
            } else {
                cout << "Test failed, there are in total " << err0 + err1 + err2 + err3 + err4 + err5 + err6
                     << " errors!" << endl;
            }
        }
    }
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    if (l_verify) {
        readBin(filePath + "imloc.bin", sizeof(float) * l_imgX * l_imgZ, imlocref);
        pass7 = compare<RTM_dataType>(l_imgX * l_imgZ, p_i.data(), imlocref.data(), err7);
        cout << "There are in total " << err7 << " errors in img v.s. img_ref" << endl;
    } else {
        pass7 = true;
        cout << "WARNING: verification is disabled." << endl;
    }

    cout << "Execution completed" << endl;
    cout << "Average forward execution time " << elapsedF / l_shot << "s." << endl;
    cout << "Average backward execution time " << elapsedB / l_shot << "s." << endl;
    cout << "Average total execution time " << elapsed.count() / l_shot << "s." << endl;

    if (pass7)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

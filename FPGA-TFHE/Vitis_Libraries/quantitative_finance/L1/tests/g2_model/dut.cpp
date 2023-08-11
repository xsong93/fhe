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

/**
 * @file dut.cpp
 *
 * @brief This file contains top function of test case.
 */

#include <ap_fixed.h>
#include <ap_int.h>
#include "hls_stream.h"
#include "g2_model.hpp"
#include "utils.hpp"
#include "trinomial_tree.hpp"
#include "ornstein_uhlenbeck_process.hpp"
#include <iostream>

#define LEN 16
#define LEN2 32
typedef double DT;
typedef xf::fintech::OrnsteinUhlenbeckProcess<DT> Process;
typedef xf::fintech::TrinomialTree<DT, Process, LEN> Tree;
typedef xf::fintech::G2Model<DT, Tree, LEN2> Model;

void dut(int endCnt,
         DT time[LEN],
         DT dtime[LEN],
         DT flatRate,
         DT spread,
         DT a,
         DT sigma,
         DT b,
         DT eta,
         DT rho,
         DT x0,
         DT* discount) {
    xf::fintech::internal::xf_2D_array<DT, 4, LEN2> tmp_values1;
    xf::fintech::internal::xf_2D_array<DT, 4, LEN2> tmp_values2;
    DT rates[LEN];

    Model model;
    Tree tree[2];
    DT process1[4] = {a, sigma, 0.0, 0.0};
    DT process2[4] = {b, eta, 0.0, 0.0};

    tree[0].initialization(process1, endCnt, x0);
    tree[1].initialization(process2, endCnt, x0);
    model.initialization(flatRate, a, sigma, b, eta, rho);
    model.treeShortRate(tree, endCnt, time, dtime, tmp_values1, tmp_values2, rates);
    DT x[2];
    x[0] = tree[0].underlying(0);
    x[1] = tree[1].underlying(0);
    *discount = model.discount(time[endCnt - 2], dtime[endCnt - 2], x, rates[endCnt - 2]);
#ifndef __SYNTHESIS__
    std::cout << "i=" << endCnt - 2 << ",x=" << x[0] << "," << x[1] << ",rates[i]=" << rates[endCnt - 2]
              << ",disc=" << *discount << std::endl;
#endif
}

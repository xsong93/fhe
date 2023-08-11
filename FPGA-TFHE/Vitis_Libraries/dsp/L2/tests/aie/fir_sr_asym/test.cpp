/*
 * Copyright 2021 Xilinx, Inc.
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
/*
This file holds the body of the test harness for single rate asymmetric FIR filter
reference model graph
*/

#include <stdio.h>
#include "test.hpp"

#if (NUM_OUTPUTS == 1)
simulation::platform<1, 1> platform(QUOTE(INPUT_FILE), QUOTE(OUTPUT_FILE));
#else
#if (DUAL_IP == 1)
// temporarily connect same file to both inputs
simulation::platform<2, 2> platform(QUOTE(INPUT_FILE), QUOTE(INPUT_FILE), QUOTE(OUTPUT_FILE), QUOTE(OUTPUT_FILE2));
#else
simulation::platform<1, 2> platform(QUOTE(INPUT_FILE), QUOTE(OUTPUT_FILE), QUOTE(OUTPUT_FILE2));
#endif
#endif

xf::dsp::aie::testcase::test_graph filter;

// Connect filter instance to platform
connect<> net0a(platform.src[0], filter.in);
#if (DUAL_IP == 1)
connect<> net0b(platform.src[1], filter.in2);
#endif
connect<> net1(filter.out, platform.sink[0]);
#if (NUM_OUTPUTS == 2)
connect<> net2(filter.out2, platform.sink[1]);
#endif

int main(void) {
    printf("\n");
    printf("========================\n");
    printf("UUT: ");
    printf(QUOTE(UUT_GRAPH));
    printf("\n");
    printf("========================\n");
    printf("Input samples   = %d \n", INPUT_SAMPLES);
    printf("Input margin    = %lu \n", INPUT_MARGIN(FIR_LEN, DATA_TYPE));
    printf("Output samples  = %d \n", OUTPUT_SAMPLES);
    printf("FIR Length      = %d \n", FIR_LEN);
    printf("Shift           = %d \n", SHIFT);
    printf("ROUND_MODE      = %d \n", ROUND_MODE);
    printf("Data type       = ");
    printf(QUOTE(DATA_TYPE));
    printf("\n");
    printf("Coeff type      = ");
    printf(QUOTE(COEFF_TYPE));
    printf("\nCoeff reload  = %d \n", USE_COEFF_RELOAD);
    printf("CASC_LEN        = %d \n", CASC_LEN);
    printf("NUM_OUTPUTS     = %d \n", NUM_OUTPUTS);
    printf("\n");

    filter.init();

#if (USE_COEFF_RELOAD == 1)
    filter.update(filter.coeff, filter.m_taps[0], FIR_LEN);
    filter.run(NITER / 2);
    filter.wait();
    filter.update(filter.coeff, filter.m_taps[1], FIR_LEN);
    filter.run(NITER / 2);
#else
    filter.run(NITER);
#endif

    filter.end();

    return 0;
}

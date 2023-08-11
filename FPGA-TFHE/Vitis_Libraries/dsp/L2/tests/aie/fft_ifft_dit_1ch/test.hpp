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
#ifndef _DSPLIB_TEST_HPP_
#define _DSPLIB_TEST_HPP_

/*
This file holds the declaraion of the test harness graph class for the
fft_ifft_dit_1ch graph class.
*/

#include <adf.h>
#include <vector>
#include "utils.hpp"

#include "uut_config.h"
#include "test_stim.hpp"

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifndef UUT_GRAPH
#define UUT_GRAPH fft_ifft_dit_1ch_graph
#endif

#include QUOTE(UUT_GRAPH.hpp)

#include "widget_api_cast_graph.hpp"

using namespace adf;

namespace xf {
namespace dsp {
namespace aie {
namespace testcase {

class test_graph : public graph {
   private:
   public:
    port<input> in[(1 + API_IO) << PARALLEL_POWER];
    port<output> out[(1 + API_IO) << PARALLEL_POWER];

    // Constructor
    test_graph() {
        printf("========================\n");
        printf("== UUT Graph Class: ");
        printf(QUOTE(UUT_GRAPH));
        printf("\n");
        printf("========================\n");
        printf("Input samples        = %d \n", INPUT_SAMPLES);
        printf("Input window (bytes) = %lu\n", INPUT_SAMPLES * sizeof(DATA_TYPE));
        printf("Output samples       = %d \n", OUTPUT_SAMPLES);
        printf("Point size           = %d \n", POINT_SIZE);
        printf("FFT/nIFFT            = %d \n", FFT_NIFFT);
        printf("Final scaling Shift  = %d \n", SHIFT);
        printf("Number of kernels    = %d \n", CASC_LEN);
        printf("Dynamic point size   = %d \n", DYN_PT_SIZE);
        printf("Window Size          = %d \n", WINDOW_VSIZE);
        printf("API_IO               = %d \n", API_IO);
        printf("PARALLEL_POWER       = %d \n", PARALLEL_POWER);
        printf("Data type            = ");
        printf(QUOTE(DATA_TYPE));
        printf("\n");
        printf("TWIDDLE type         = ");
        printf(QUOTE(TWIDDLE_TYPE));
        printf("\n");

        printf("========================\n");

        // FIR sub-graph
        xf::dsp::aie::fft::dit_1ch::UUT_GRAPH<DATA_TYPE, TWIDDLE_TYPE, POINT_SIZE, FFT_NIFFT, SHIFT, CASC_LEN,
                                              DYN_PT_SIZE, WINDOW_VSIZE, API_IO, PARALLEL_POWER>
            fftGraph;
        if (API_IO == 1) {                                    // dual streams for input
            for (int i = 0; i < (2 << PARALLEL_POWER); i++) { // 2? 2 streams per subframe FFT
                connect<stream>(in[i], fftGraph.in[i]);   // window<(WINDOW_VSIZE*sizeof(DATA_TYPE)>>PARALLEL_POWER)>
                connect<stream>(fftGraph.out[i], out[i]); // avoiding multiple drivers by being outside the loop
            }
        } else { // native single window input to FFT
            // Make connections
            // Size of window in Bytes.
            connect<>(in[0], fftGraph.in[0]);
            connect<>(fftGraph.out[0], out[0]);
        }
    };
};
}
}
}
};

#endif // _DSPLIB_TEST_HPP_

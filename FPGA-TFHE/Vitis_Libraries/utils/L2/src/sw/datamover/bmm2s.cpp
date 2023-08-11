/*
 * Copyright 2020 Xilinx, Inc.
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

#include <stdint.h>
#include "xf_datamover/load_master_to_stream.hpp"

extern "C" void bmm2s(
    // 0
    ap_uint<32>* DataIn2,
    hls::stream<ap_axiu<32, 0, 0, 0> >& s,
    uint64_t sz0

    ) {
    using namespace xf::datamover;

    ; // clang-format off
#pragma HLS interface m_axi offset=slave bundle=gmem0 port=DataIn2 \
    max_read_burst_length=32 num_read_outstanding=4 latency=128
#pragma HLS interface s_axilite bundle=control port=DataIn2
#pragma HLS interface axis port=s
#pragma HLS interface s_axilite bundle=control port=sz0

#pragma HLS interface s_axilite bundle=control port=return
    ; // clang-format on

#pragma HLS dataflow

    loadMasterToStream(DataIn2, s, sz0);
}

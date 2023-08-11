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

#include "xf_gaincontrol_config.h"

static constexpr int __XF_DEPTH = (HEIGHT * WIDTH * (XF_PIXELWIDTH(IN_TYPE, NPC1)) / 8) / (INPUT_PTR_WIDTH / 8);

extern "C" {

void gaincontrol_accel(ap_uint<INPUT_PTR_WIDTH>* img_inp,
                       ap_uint<OUTPUT_PTR_WIDTH>* img_out,
                       int rows,
                       int cols,
                       unsigned short rgain,
                       unsigned short bgain) {
// clang-format off
    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem1 
    #pragma HLS INTERFACE m_axi     port=img_out  offset=slave bundle=gmem2 
    
    #pragma HLS INTERFACE s_axilite port=rows     
    #pragma HLS INTERFACE s_axilite port=cols
	#pragma HLS INTERFACE s_axilite port=rgain     
    #pragma HLS INTERFACE s_axilite port=bgain	
    #pragma HLS INTERFACE s_axilite port=return
    // clang-format on

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> in_mat(rows, cols);
    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> _dst(rows, cols);

// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPC1>(img_inp, in_mat);

    xf::cv::gaincontrol<BFORMAT, IN_TYPE, HEIGHT, WIDTH, NPC1>(in_mat, _dst, rgain, bgain);

    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPC1>(_dst, img_out);
}
}

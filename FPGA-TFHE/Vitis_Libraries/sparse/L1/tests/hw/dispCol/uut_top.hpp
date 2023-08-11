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
#ifndef XF_SPARSE_UUT_TOP_HPP
#define XF_SPARSE_UUT_TOP_HPP

#include "xf_sparse.hpp"

void uut_top(hls::stream<ap_uint<SPARSE_dataBits * SPARSE_parEntries> >& p_datStr,
             hls::stream<ap_uint<SPARSE_dataBits * SPARSE_parEntries> > p_datOutStr[SPARSE_hbmChannels]);

#endif

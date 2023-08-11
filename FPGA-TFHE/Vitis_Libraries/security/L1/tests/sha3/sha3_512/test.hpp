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
#ifndef _TEST_H_
#define _TEST_H_

#include <ap_int.h>
#include "hls_stream.h"

void test(hls::stream<ap_uint<64> >& msgStrm,
          hls::stream<ap_uint<128> >& msgLenStrm,
          hls::stream<bool>& endMsgLenStrm,
          hls::stream<ap_uint<512> >& digestStrm,
          hls::stream<bool>& endDigestStrm);
#endif

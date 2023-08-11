/*
 * (c) Copyright 2019-2021 Xilinx, Inc. All rights reserved.
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
 *
 */
#ifndef _XFCOMPRESSION_CHECKSUM_MM_HPP_
#define _XFCOMPRESSION_CHECKSUM_MM_HPP_

/**
 * @file checksum_mm.hpp
 * @brief Header for Checksum Kernel.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "hls_stream.h"
#include <ap_int.h>

#include "checksum_wrapper.hpp"

#ifndef PARALLEL_BYTES
#define PARALLEL_BYTES 16
#endif

// Kernel top functions
extern "C" {
/**
 * @brief Checksum kernel takes the raw data as input and generates the
 * checksum result.
 *
 * @param in input raw data
 * @param initData input Initial data
 * @param inSize input size
 * @param checksumType CRC/ADLER
 */
void xilChecksum32(const ap_uint<PARALLEL_BYTES * 8>* in, ap_uint<32>* initData, uint32_t inSize, bool checksumType);
}
#endif // _XF_COMPRESSION_CHECKSUM_MM_HPP_

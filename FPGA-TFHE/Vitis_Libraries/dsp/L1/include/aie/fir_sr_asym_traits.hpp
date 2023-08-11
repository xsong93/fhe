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
#ifndef _DSPLIB_FIR_SR_ASYM_TRAITS_HPP_
#define _DSPLIB_FIR_SR_ASYM_TRAITS_HPP_

/*
Single Rate Asymetrical FIR traits.
This file contains sets of overloaded, templatized and specialized templatized functions which
encapsulate properties of the intrinsics used by the main kernal class. Specifically,
this file does not contain any vector types or intrinsics since it is required for construction
and therefore must be suitable for the aie compiler graph-level compilation.
*/

#define NOT_SUPPORTED 0
#define SUPPORTED 1

namespace xf {
namespace dsp {
namespace aie {
namespace fir {
namespace sr_asym {
enum eArchType { kArchBasic = 0, kArchIncLoads, kArchZigZag, kArchStream };

static constexpr unsigned int kMaxColumns = 2;
static constexpr unsigned int kUpdWSize = 32;         // Upd_w size in Bytes (256bit) - const for all data/coeff types
static constexpr unsigned int kBuffSize128Byte = 128; // 1024-bit buffer size in Bytes
static constexpr unsigned int kBuffSize64Byte = 64;   // 512-bit buffer size in Bytes
static constexpr unsigned int kBuffSize32Byte = 32;   // 256-bit buffer size in Bytes

// align to 256b for FIR cascade splitting - also should be a multiple of lanes/columns
template <typename TT_DATA>
inline constexpr unsigned int fnFirRangeRound() {
    return ((258 / 8) / sizeof(TT_DATA));
}

// Calculate ASYM FIR range for cascaded kernel
template <unsigned int TP_FL, unsigned int TP_CL, int TP_KP, typename TT_DATA, unsigned int TP_API>
inline constexpr unsigned int fnFirRangeAsym() {
    // TP_FL - FIR Length, TP_CL - Cascade Length, TP_KP - Kernel Position
    // make sure there's no runt filters ( lengths < 4)
    // make each cascade rounded to fnFirRangeRound and only last in the chain possibly odd
    // Limited to stream architectures for now
    return fnFirRange<TP_FL, TP_CL, TP_KP, ((TP_API == 1) ? (fnFirRangeRound<TT_DATA>()) : 1)>();
}
template <unsigned int TP_FL, unsigned int TP_CL, int TP_KP, typename TT_DATA, unsigned int TP_API>
inline constexpr unsigned int fnFirRangeRemAsym() {
    // TP_FL - FIR Length, TP_CL - Cascade Length, TP_KP - Kernel Position
    // this is for last in the cascade
    return fnFirRangeRem<TP_FL, TP_CL, TP_KP, ((TP_API == 1) ? (fnFirRangeRound<TT_DATA>()) : 1)>();
}

// Calculate ASYM FIR range offset for cascaded kernel
template <unsigned int TP_FL, unsigned int TP_CL, int TP_KP, typename TT_DATA, unsigned int TP_API>
inline constexpr unsigned int fnFirRangeOffsetAsym() {
    // TP_FL - FIR Length, TP_CL - Cascade Length, TP_KP - Kernel Position
    return fnFirRangeOffset<TP_FL, TP_CL, TP_KP, ((TP_API == 1) ? (fnFirRangeRound<TT_DATA>()) : 1)>();
}

// function to return the number of lanes for a type combo
template <typename TT_DATA, typename TT_COEFF>
inline constexpr unsigned int fnNumLanesSrAsym() {
    return 0; // effectively an error trap, but adding an error message to a constexpr return results in a warning.
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<int16, int16>() {
    return 16;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint16, int16>() {
    return 8;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint16, cint16>() {
    return 8;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<int32, int16>() {
    return 8;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<int32, int32>() {
    return 8;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint32, int16>() {
    return 4;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint32, cint16>() {
    return 4;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint32, int32>() {
    return 4;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cint32, cint32>() {
    return 2;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<float, float>() {
    return 8;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cfloat, float>() {
    return 4;
};
template <>
inline constexpr unsigned int fnNumLanesSrAsym<cfloat, cfloat>() {
    return 4;
};

// function to return the number of columns for a type combo for the intrinsics used for this single rate asym FIR
template <typename TT_DATA, typename TT_COEFF>
inline constexpr unsigned int fnNumColumnsSrAsym() {
    return sizeof(TT_COEFF) == 2 ? 2 : 1;
};
// specialize for any exceptions like this:
// template<> inline constexpr unsigned int fnNumColumnsDecHb< int16,  int16, K_ARCH_1BUFF>() { return 2;};

template <typename TT_DATA>
inline constexpr unsigned int fnDataLoadsInRegSrAsym() {
    // To fill a full 1024-bit input vector register using a 256-bit upd_w command it takes 4 load operations.
    // Always return 4, the exception (handled by explicit template instantiation)
    // would be needed it a different command was used (e.g. upd_v, upd_x).
    return 4;
}

// Parameter to constant resolution functions
template <typename TT_DATA>
inline constexpr unsigned int fnDataLoadVsizeSrAsym() {
    return (kUpdWSize / sizeof(TT_DATA));
}

// Helper function to define max ZigZag Coefficient array
template <typename TT_COEFF>
inline constexpr unsigned int fnZigZagMaxCoeffByteSize() {
    return 256;
}

// function to return the number of samples in an output vector for a type combo
template <typename TT_DATA, typename TT_COEFF>
inline constexpr unsigned int fnVOutSizeSrAsym() {
    return fnNumLanesSrAsym<TT_DATA, TT_COEFF>();
};

// Support for stream interface for a give data/coeff type combination
template <typename TT_DATA, typename TT_COEFF, unsigned int TP_API>
inline constexpr unsigned int fnTypeStreamSupport() {
    return SUPPORTED;
};
// Exclude cint32/cint32 combo.
template <>
inline constexpr unsigned int fnTypeStreamSupport<cint32, cint32, 1>() {
    return NOT_SUPPORTED;
};
}
}
}
}
}
#endif // _DSPLIB_FIR_SR_ASYM_TRAITS_HPP_

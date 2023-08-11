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
#ifndef _DSPLIB_FIR_SR_SYM_REF_GRAPH_HPP_
#define _DSPLIB_FIR_SR_SYM_REF_GRAPH_HPP_

/*
This file holds the declaration of the reference model
graph class for the Single Rate Symmetrical FIR filter.
*/
#include <adf.h>
#include <vector>
#include "fir_sr_sym_ref.hpp"
#include "fir_ref_utils.hpp"

namespace xf {
namespace dsp {
namespace aie {
namespace fir {
namespace sr_sym {
using namespace adf;

template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_CASC_LEN = 1,
          unsigned int TP_USE_COEFF_RELOAD = 0,
          unsigned int TP_NUM_OUTPUTS = 1,
          unsigned int TP_API = 0>
class fir_sr_sym_ref_graph : public graph {
   public:
    port<input> in;
    port<output> out;

    // FIR Kernel
    kernel m_firKernel;

    // Constructor
    fir_sr_sym_ref_graph(const std::vector<TT_COEFF>& taps) {
        printf("========================\n");
        printf("== FIR SR SYM REF Graph\n");
        printf("========================\n");

        // Create FIR class
        m_firKernel = kernel::create_object<
            fir_sr_sym_ref<TT_DATA, TT_COEFF, TP_FIR_LEN, TP_SHIFT, TP_RND, TP_INPUT_WINDOW_VSIZE, 0, 1, TP_API> >(
            taps);

        // Make connections
        // Size of window in Bytes.
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA), fnFirMargin<TP_FIR_LEN, TT_DATA>() * sizeof(TT_DATA)> >(
            in, m_firKernel.in[0]);
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[0], out);

        // Specify mapping constraints
        runtime<ratio>(m_firKernel) = 0.4;

        // Source files
        source(m_firKernel) = "fir_sr_sym_ref.cpp";
    };
};

// Specialization for static coeffs and dual outputs
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_CASC_LEN,
          unsigned int TP_API>
class fir_sr_sym_ref_graph<TT_DATA,
                           TT_COEFF,
                           TP_FIR_LEN,
                           TP_SHIFT,
                           TP_RND,
                           TP_INPUT_WINDOW_VSIZE,
                           TP_CASC_LEN,
                           0,
                           2,
                           TP_API> : public graph {
   public:
    port<input> in;
    port<output> out;
    port<output> out2;

    // FIR Kernel
    kernel m_firKernel;

    // Constructor
    fir_sr_sym_ref_graph(const std::vector<TT_COEFF>& taps) {
        printf("========================\n");
        printf("== FIR SR SYM REF Graph\n");
        printf("========================\n");

        // Create FIR class
        m_firKernel = kernel::create_object<
            fir_sr_sym_ref<TT_DATA, TT_COEFF, TP_FIR_LEN, TP_SHIFT, TP_RND, TP_INPUT_WINDOW_VSIZE, 0, 2, TP_API> >(
            taps);

        // Make connections
        // Size of window in Bytes.
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA), fnFirMargin<TP_FIR_LEN, TT_DATA>() * sizeof(TT_DATA)> >(
            in, m_firKernel.in[0]);
        if (TP_API == 0) {
            // Complete set of outputs are copied into 2 window buffers.
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[0], out);
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[1], out2);
        } else {
            // Set of outputs is split between window buffers. Hence, halft the window size.
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA) / 2> >(m_firKernel.out[0], out);
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA) / 2> >(m_firKernel.out[1], out2);
        }

        // Specify mapping constraints
        runtime<ratio>(m_firKernel) = 0.4;

        // Source files
        source(m_firKernel) = "fir_sr_sym_ref.cpp";
    };
};

// Specialization for reload coefficients and single output
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_CASC_LEN,
          unsigned int TP_API>
class fir_sr_sym_ref_graph<TT_DATA,
                           TT_COEFF,
                           TP_FIR_LEN,
                           TP_SHIFT,
                           TP_RND,
                           TP_INPUT_WINDOW_VSIZE,
                           TP_CASC_LEN,
                           1,
                           1,
                           TP_API> : public graph {
   public:
    port<input> in;
    port<output> out;
    port<input> coeff;

    // FIR Kernel
    kernel m_firKernel;

    // Constructor
    fir_sr_sym_ref_graph() {
        printf("========================\n");
        printf("== FIR SR SYM REF Graph\n");
        printf("========================\n");

        // Create FIR class
        m_firKernel = kernel::create_object<
            fir_sr_sym_ref<TT_DATA, TT_COEFF, TP_FIR_LEN, TP_SHIFT, TP_RND, TP_INPUT_WINDOW_VSIZE, 1, 1, TP_API> >();

        // Make connections
        // Size of window in Bytes.
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA), fnFirMargin<TP_FIR_LEN, TT_DATA>() * sizeof(TT_DATA)> >(
            in, m_firKernel.in[0]);
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[0], out);
        connect<parameter>(coeff, async(m_firKernel.in[1]));

        // Specify mapping constraints
        runtime<ratio>(m_firKernel) = 0.4;

        // Source files
        source(m_firKernel) = "fir_sr_sym_ref.cpp";
    };
};

// Specialization for reload coefficients and dual output
template <typename TT_DATA,
          typename TT_COEFF,
          unsigned int TP_FIR_LEN,
          unsigned int TP_SHIFT,
          unsigned int TP_RND,
          unsigned int TP_INPUT_WINDOW_VSIZE,
          unsigned int TP_CASC_LEN,
          unsigned int TP_API>
class fir_sr_sym_ref_graph<TT_DATA,
                           TT_COEFF,
                           TP_FIR_LEN,
                           TP_SHIFT,
                           TP_RND,
                           TP_INPUT_WINDOW_VSIZE,
                           TP_CASC_LEN,
                           1,
                           2,
                           TP_API> : public graph {
   public:
    port<input> in;
    port<output> out;
    port<output> out2;
    port<input> coeff;

    // FIR Kernel
    kernel m_firKernel;

    // Constructor
    fir_sr_sym_ref_graph() {
        printf("========================\n");
        printf("== FIR SR SYM REF Graph\n");
        printf("========================\n");

        // Create FIR class
        m_firKernel = kernel::create_object<
            fir_sr_sym_ref<TT_DATA, TT_COEFF, TP_FIR_LEN, TP_SHIFT, TP_RND, TP_INPUT_WINDOW_VSIZE, 1, 2, TP_API> >();

        // Make connections
        // Size of window in Bytes.
        connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA), fnFirMargin<TP_FIR_LEN, TT_DATA>() * sizeof(TT_DATA)> >(
            in, m_firKernel.in[0]);
        if (TP_API == 0) {
            // Complete set of outputs are copied into 2 window buffers.
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[0], out);
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA)> >(m_firKernel.out[1], out2);
        } else {
            // Set of outputs is split between window buffers. Hence, halft the window size.
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA) / 2> >(m_firKernel.out[0], out);
            connect<window<TP_INPUT_WINDOW_VSIZE * sizeof(TT_DATA) / 2> >(m_firKernel.out[1], out2);
        }
        connect<parameter>(coeff, async(m_firKernel.in[1]));

        // Specify mapping constraints
        runtime<ratio>(m_firKernel) = 0.4;

        // Source files
        source(m_firKernel) = "fir_sr_sym_ref.cpp";
    };
};
}
}
}
}
}
#endif // _DSPLIB_FIR_SR_SYM_REF_GRAPH_HPP_

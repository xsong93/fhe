/*
 * Copyright 2021 Xilinx, Inc.
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

#ifndef __DUT_QRF_H__
#define __DUT_QRF_H__

#include "hls_x_complex.h"
#include "utils/x_hls_traits.h"
#include "dut_type_float.hpp"
//#include "dut_type_complex.hpp"
#include <limits>

template <typename T>
struct testbench_traits {
    typedef unsigned ULP_T;
    typedef double A_STIMULI_T;
    typedef T BASE_T;

    // // I need this to transpose a real matrix.  For complex testbenches, I need to pass
    // // CONJUGATE so this sets the correct value at compile time.
    // typedef hls::matrix_transpose_type<T> __TRANSPOSE__T_;
    // static const typename __TRANSPOSE__T_::transpose_t TRANSPOSE_TYPE = __TRANSPOSE__T_::TRANSPOSE;

    // I need to set the type for L_expected_raw, which is the output of the LAPACK functions.
    // If T is fixed point then it is to be of type float, else T
};

template <typename T>
struct testbench_traits<hls::x_complex<T> > {
    typedef hls::x_complex<unsigned> ULP_T;
    typedef hls::x_complex<double> A_STIMULI_T;
    typedef T BASE_T;

    // // I need this to conjugate transpose a complex matrix
    // typedef hls::matrix_transpose_type<hls::x_complex<T> > __TRANSPOSE__T_;
    // static const typename __TRANSPOSE__T_::transpose_t TRANSPOSE_TYPE = __TRANSPOSE__T_::CONJUGATE;
};

template <typename T>
struct testbench_traits<std::complex<T> > {
    typedef std::complex<unsigned> ULP_T;
    typedef std::complex<double> A_STIMULI_T;
    typedef T BASE_T;

    // // I need this to conjugate transpose a complex matrix
    // typedef hls::matrix_transpose_type<std::complex<T> > __TRANSPOSE__T_;
    // static const typename __TRANSPOSE__T_::transpose_t TRANSPOSE_TYPE = __TRANSPOSE__T_::CONJUGATE;
};
// I need to set the type for L_expected_raw, which is the output of the LAPACK functions.
// If T is fixed point then it is to be of type float, else T

template <typename T>
struct lapack_interface {
    typedef T QR_TYPE;
    typedef T QR_BASE_TYPE;

    static void identify() { printf(" template <typename T> struct lapack_interface\n"); }
};

template <typename T>
struct lapack_interface<hls::x_complex<T> > {
    typedef hls::x_complex<T> QR_TYPE;
    typedef T QR_BASE_TYPE;

    static void identify() { printf(" template <typename T> struct lapack_interface< hls::x_complex<T>\n"); }
};

template <typename T>
struct lapack_interface<std::complex<T> > {
    typedef std::complex<T> QR_TYPE;
    typedef T QR_BASE_TYPE;

    static void identify() { printf(" template <typename T> struct lapack_interface< std::complex<T>\n"); }
};
template <int W, int I, ap_q_mode Q, ap_o_mode O, int N>
struct lapack_interface<ap_fixed<W, I, Q, O, N> > {
    typedef double QR_TYPE;
    typedef double QR_BASE_TYPE;

    static void identify() { printf(" template <int W, int I> struct lapack_interface<ap_fixed<int W, int I> >{\n"); }
};

template <int W, int I, ap_q_mode Q, ap_o_mode O, int N>
struct lapack_interface<hls::x_complex<ap_fixed<W, I, Q, O, N> > > {
    typedef hls::x_complex<double> QR_TYPE;
    typedef double QR_BASE_TYPE;

    static void identify() {
        printf(" template <int W, int I> struct lapack_interface< hls::x_complex<ap_fixed<W, I> > >{\n");
    }
};
template <int W, int I, ap_q_mode Q, ap_o_mode O, int N>
struct lapack_interface<std::complex<ap_fixed<W, I, Q, O, N> > > {
    typedef std::complex<double> QR_TYPE;
    typedef double QR_BASE_TYPE;

    static void identify() {
        printf(" template <int W, int I> struct lapack_interface< std::complex<ap_fixed<W, I> > >{\n");
    }
};

typedef testbench_traits<MATRIX_IN_T>::BASE_T MATRIX_IN_BASE_T;
typedef lapack_interface<MATRIX_OUT_T>::QR_TYPE QR_TYPE;
typedef lapack_interface<MATRIX_OUT_T>::QR_BASE_TYPE QR_BASE_TYPE;

const unsigned int NUM_MAT_TYPES = 9;
const QR_BASE_TYPE eps = hls::numeric_limits<MATRIX_IN_BASE_T>::epsilon();

#endif

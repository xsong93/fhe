..
   Copyright 2021 Xilinx, Inc.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

.. meta::
   :keywords: QRF
   :description: QR Factorization
   :xlnxdocumentclass: Document
   :xlnxdocumenttype: Tutorials

*******************************************************
QRF (QR Factorization)
*******************************************************

Overview
============
QRF, also known as QR decomposition, is a decomposition of a matrix :math:`A` into a product of an orthogonal matrix :math:`Q` and an upper triangular matrix :math:`R`. 

QRF is often used to solve the linear least squares problem and is the basis for a particular eigenvalue algorithm, the QR algorithm.

.. math::
            A = Q R

There are several methods for actually computing the QR decomposition, such as by means of the Gram-Schmidt process, Householder transformations, or Givens rotations. Each has a number of advantages and disadvantages. For more details, please refer: `QR_decomposition <https://en.wikipedia.org/wiki/QR_decomposition>`_.

In our design, Given rotations is used.


Implementation
============

DataType Supported
--------------------
* float
* x_complex<float>

.. note::
   Subnormall values are not supported. If used, the synthesized hardware will flush these to zero, and the behavior will differ versus software simulation.

Interfaces
--------------------
* Template parameters:

  * TransposedQ      : Selects whether Q is output in transposed form
  * RowsA            : Number of rows in input matrix A
  * ColsA            : Number of columns in input matrix A
  * InputType        : Input data type
  * OutputType       : Output data type

* Arguments:

  * A                : Input matrix
  * Q                : Orthogonal output matrix
  * R                : Upper triangular output matrix

.. note::
   The function will failed to compile or synthesize if **RowsA < ColsA**.


Implementation Controls
------------------------

Specifications
~~~~~~~~~~~~~~~~~~~~~~~~~
There is a configuration class derived from the base configuration class **`xf::solver::qrf_traits** by redefining the appropriate class member.

.. code::
   struct my_qrf_traits : xf::solver::qrf_traits<A_ROWS, A_COLS, MATRIX_IN_T, MATRIX_OUT_T> {
       static const int ARCH = SEL_ARCH;
   };

The base configuration class is:

.. code::
   template <int RowsA, int ColsA, typename InputType, typename OutputType>
   struct qrf_traits {
       static const int ARCH = 1;         
       static const int CALC_ROT_II = 1; 
       static const int UPDATE_II = 4;    
       static const int UNROLL_FACTOR =1; 
   };

.. note::
   * ARCH:          Select implementation. 0=Basic. 1=Lower latency/thoughput architecture.
   * CALC_ROT_II:   Specify the rotation calculation loop target II of the QRF_ALT architecture(1).
   * UPDATE_II:     Specify the pipelining target for the Q & R update loops.
   * UNROLL_FACTOR: Specify the unrolling factor for Q & R update loops of the QRF_ALT architecture(1).

The configuration class is supplied to the **xf::solver::qrf_top** function as a template paramter as follows.

.. code::
   template <bool TransposedQ, int RowsA, int ColsA, typename InputType, typename OutputType>
   void qrf(const InputType A[RowsA][ColsA], OutputType Q[RowsA][RowsA], OutputType R[RowsA][ColsA]) {
       typedef qrf_traits<RowsA, ColsA, InputType, OutputType> DEFAULT_QRF_TRAITS;
    qrf_top<TransposedQ, RowsA, ColsA, DEFAULT_QRF_TRAITS, InputType, OutputType>(A, Q, R);
   }


Key Factors
~~~~~~~~~~~~~~~~~~~~~~~~~
The following table summarizes that how the key factors which from the configuration class influence resource utilization, function throughput (initiation interval), and function latency. The values of Low, Medium, and High are relative to the other key factors.

.. table:: QRF Key Factor Summary  
    :align: center

    +------------------+-------+-----------+------------+----------+
    |    Key Factor    | Value | Resources | Throughput | Latency  |
    +==================+=======+===========+============+==========+
    | Q and R update   |   2   |   High    |    High    |  Low     |    
    | loop pipelining  +-------+-----------+------------+----------+    
    | (UPDATE_II)      |   >2  |   Low     |    Low     |  High    |    
    +------------------+-------+-----------+------------+----------+
    | Q and R update   |   1   |   Low     |    Low     |  High    |    
    | unrolling        +-------+-----------+------------+----------+    
    | (UNROLL_FACTOR)  |   >1  |   High    |    High    |  Low     |    
    +------------------+-------+-----------+------------+----------+
    | Rotation loop    |   1   |   High    |    High    |  Low     |    
    | pipelining       +-------+-----------+------------+----------+    
    | (CALC_ROT_II)    |   >1  |   Low     |    Low     |  High    |    
    +------------------+-------+-----------+------------+----------+

.. Note::
  * Q and R update loop pipelining: Sets the achievable initiation interval (II); 
  * Q and R update loop unrolling:  Duplicate hardware when implement loop processing, execute corresponding number of loop iterations in parallel;
  * Rotation loop pipelining:       Enables Vivado HLS to share resources and reduce the DSP utilization



Profiling
-----------


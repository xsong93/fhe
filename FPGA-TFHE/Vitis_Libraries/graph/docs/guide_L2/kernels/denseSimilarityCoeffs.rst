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


*****************************************************
Internal Design of Dense Similarity with Coefficient
*****************************************************

Interface
===========
The input should be a set of integer vertex list with known size.
The result will return a vertex list with each vertex corresponding similarity value.
For optimizing the calculation of dense and integer value, the design disable neither float datatype nor sparse mode. Further more, the support for Jaccard Similarity is also removed in the kernel. So that it save lots of hardware resourse and realize a design of 2-CU instantiation to get the best performance on the platform of U55C. The design support additional coefficients for each column of weight for a better software flexibility.

.. image:: /images/dense_similarity_coefficient_formula.PNG
   :alt:  Formula of Dense Similarity with Coefficient
   :width: 65%
   :align: center

Implemention
============

The detail algorithm implemention is illustrated as below:

.. image:: /images/dense_similarity_coefficient_internal.PNG
   :alt: Diagram of Dense Similarity
   :width: 70%
   :align: center

In the calculation of dense similarity, most of internal loop size is set by the config variables, so that the reference vertex is alligned with others. The source vertex is initialized by multiplying the value of coefficient. Only integer value can be processed in the kernel, and all the calculation is using LUT arethmatics. In the integer version, the 32-bit input will be accumulated by 64-bit registers, and the output float similarity is divide result of two 64-bit integers.
The overall diagram of dense similarity kernel have a insert sort module which return the top K number of similarity values.
The maximum number of K is a template number which can be changed by rebuilding the xclbin. The default value of top K is 32.

Profiling and Benchmarks
========================

The kernel is validated on Alveo U55C board at 220MHz frequency. 
The hardware resource utilization and benchmark results are shown in the two table below.

.. table:: Table 1 Hardware resources
    :align: center

    +------------------------+--------------+----------------+----------+----------+--------+
    |          Name          |      LUT     |    Register    |   BRAM   |   URAM   |   DSP  |
    +------------------------+--------------+----------------+----------+----------+--------+
    |  denseSimilarityKernel |    262317    |    233100      |    794   |    48    |    9   |
    |  (int + 2CU + Coeffs)  |              |                |          |          |        |
    +------------------------+--------------+----------------+----------+----------+--------+


.. table:: Table 2 Performance comparison of dense graph between TigerGraph on CPU and FPGA
    :align: center
    
    +------------------+----------+----------+-----------------+----------------+------------------------------+
    |                  |          |          |                 |                |  TigerGraph (32 core 512 GB) |
    |     Datasets     |  Vertex  |   Edges  | Similarity Type | FPGA Time / ms +----------------+-------------+
    |                  |          |          |                 |                |   Time / ms    |  Speed up   |
    +------------------+----------+----------+-----------------+----------------+----------------+-------------+
    | Patients(1GB/CU) | 1250000  |   200    |      Cosine     |    7.0         |    585.7       |    83.5     |
    +------------------+----------+----------+-----------------+----------------+----------------+-------------+
    

.. note::
    | 1. Tigergraph running on platform with Intel(R) Xeon(R) CPU E5-2640 v3 @2.600GHz, 32 Threads (16 Core(s)).
    | 2. The uint + float version and integer version have relatively similar performance. 
    | 3. Time unit: ms.

.. toctree::
    :maxdepth: 1

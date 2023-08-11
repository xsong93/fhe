.. 
   Copyright 2019 Xilinx, Inc.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

.. _l2_manual_pagerank:

========
PageRank
========

Pagerank example resides in ``L2/benchmarks/pagerank`` directory. The tutorial provides a step-by-step guide that covers commands for building and running kernel.

Executable Usage
===============

* **Work Directory(Step 1)**

The steps for library download and environment setup can be found in :ref:`l2_vitis_graph`. For getting the design,

.. code-block:: bash

   cd L2/benchmarks/pagerank

* **Build kernel(Step 2)**

Run the following make command to build your XCLBIN and host binary targeting a specific device. Please be noticed that this process will take a long time, maybe couple of hours.

.. code-block:: bash

   make run TARGET=hw DEVICE=xilinx_u50_gen3x16_xdma_201920_3

* **Run kernel(Step 3)**

To get the benchmark results, please run the following command.

.. code-block:: bash

   ./build_dir.hw.xilinx_u50_gen3x16_xdma_201920_3/host.exe -xclbin build_dir.hw.xilinx_u50_gen3x16_xdma_201920_3/kernel_pagerank_0.xclbin -dataSetDir data/ -refDir data/

Pagerank Input Arguments:

.. code-block:: bash

   Usage: host.exe -[-xclbin -dataSetDir -refDir]
          -xclbin:      the kernel name
          -dataSetDir:  the path point to input directory
          -refDir:      the path point to reference directory

Note: Default arguments are set in Makefile, you can use other :ref:`datasets` listed in the table.

* **Example output(Step 4)** 

.. code-block:: bash

   Found Platform
   Platform Name: Xilinx
   INFO: Found Device=xilinx_u50_gen3x16_xdma_201920_3
   INFO: Importing build_dir.hw.xilinx_u50_gen3x16_xdma_201920_3/kernel_pagerank_0.xclbin
   Loading: 'build_dir.hw.xilinx_u50_gen3x16_xdma_201920_3/kernel_pagerank_0.xclbin'
   INFO: Kernel has been created
   INFO: Finish kernel setup
   ...

   INFO: Finish kernel execution
   INFO: Finish E2E execution
   INFO: Data transfer from host to device: 240 us
   INFO: Data transfer from device to host: 106 us
   INFO: Average kernel execution per run: 5763 us
   INFO: Average execution per run: 6109 us
   ...

   INFO: sum_golden = 4.30706
   INFO: sum_pagerank = 4.30706
   INFO: Accurate Rate = 1
   INFO: Err Geomean = 8.74996e-06
   INFO: Result is correct

Profiling
=========

The hardware resource utilizations are listed in the following table.
Different tool versions may result slightly different resource.


.. table:: Table 1 Hardware resources for PageRank with a small cache (cache size 512bits)
    :align: center

    +-------------------+----------+----------+----------+----------+---------+-----------------+
    |    Kernel         |   BRAM   |   URAM   |    DSP   |    FF    |   LUT   | Frequency(MHz)  |
    +-------------------+----------+----------+----------+----------+---------+-----------------+
    | kernel_pagerank_0 |   216    |     0    |    42    |  123998  |  88372  |       300       |
    +-------------------+----------+----------+----------+----------+---------+-----------------+


.. table:: Table 2 Hardware resources for PageRank with cache (maximum supported cache size 32K in one SLR of Alveo U50)
    :align: center

    +-------------------+----------+----------+----------+----------+---------+-----------------+
    |    Kernel         |   BRAM   |   URAM   |    DSP   |    FF    |   LUT   | Frequency(MHz)  |
    +-------------------+----------+----------+----------+----------+---------+-----------------+
    | kernel_pagerank_0 |   216    |    224   |    42    |  124054  |  95950  |       225       |
    +-------------------+----------+----------+----------+----------+---------+-----------------+

With the increase of cache depth, the acceleration ratio increases obviously, but due to the use of a lot of URAM, the frequency will drop. So the adviced cache depth is 32K for 1SLR of Alveo U50.


.. table:: Table 3 Comparison between CPU SPARK and FPGA VITIS_GRAPH

    +------------------+----------+----------+-----------+-----------+----------------------------------+----------------------------------+----------------------------------+----------------------------------+
    |                  |          |          |           |           |          Spark (4 threads)       |         Spark (8 threads)        |         Spark (16 threads)       |         Spark (32 threads)       |
    |                  |          |          | FPGA time | FPGA time +------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | datasets         | Vertex   | Edges    | cache 1   | cache 32K | Spark time |  speedup | speedup  | Spark time |  speedup | speedup  | Spark time |  speedup | speedup  | Spark time |  speedup | speedup  |
    |                  |          |          |           |           |            |  Cache 1 | Cache 32K|            |  Cache 1 | Cache 32K|            |  Cache 1 | Cache 32K|            |  Cache 1 | Cache 32K|
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | as-Skitter       | 1694616  | 11094209 |   8.723   |   3.786   |  25.431    |  2.915   |  6.717   |  23.064    |   2.644  |   6.092  |   25.163   |   2.885  |   6.646  |   48.137   |   5.518  |   12.714 |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | coPapersDBLP     | 540486   | 15245729 |   6.523   |   4.217   |  29.366    |  4.502   |  6.964   |  23.56     |   3.612  |   5.587  |   27.756   |   4.255  |   6.582  |   58.432   |   8.958  |   13.856 |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | coPapersCiteseer | 434102   | 16036720 |   5.571   |   4.166   |  24.161    |  4.337   |  5.800   |  21.274    |   3.819  |   5.107  |   24.545   |   4.406  |   5.892  |   55.312   |   9.929  |   13.277 |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | cit-Patents      | 3774768  | 16518948 |  14.124   |  12.358   |  41.103    |  2.910   |  3.326   |  33.61     |   2.380  |   2.720  |   30.238   |   2.141  |   2.447  |   40.201   |   2.846  |   3.253  |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | europe_osm       | 50912018 | 54054660 |  47.376   |  51.919   | 1197.746   | 25.282   | 23.070   | 668.923    |  14.119  |  12.884  |  423.886   |   8.947  |   8.164  |     -      |     -    |     -    |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | hollywood        | 1139905  | 57515616 |  66.782   |  19.999   |  98.685    |  1.478   |  4.934   |  77.557    |   1.161  |   3.878  |   78.66    |   1.178  |   3.933  |  146.719   |   2.197  |   7.336  |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | soc-LiveJournal1 | 4847571  | 68993773 | 142.526   |  79.792   |  403.137   |  2.829   |  5.052   | 288.605    |   2.025  |   3.617  |  281.886   |   1.978  |   3.533  |  272.344   |   1.911  |   3.413  |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | ljournal-2008    | 5363260  | 79023142 | 166.998   |  66.814   |  447.311   |  2.679   |  6.695   | 258.133    |   1.546  |   3.864  |  208.849   |   1.251  |   3.126  |  281.81    |   1.688  |   4.218  |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+
    | GEOMEAN          |          |          |  27.604   |  16.121   |  105.891   |  3.837X  |  6.571X  |  78.899    |   2.858X |   4.896X |   75.152   |   2.723X |   4.663X |   95.115   |   3.772X |   6.976X |
    +------------------+----------+----------+-----------+-----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+------------+----------+----------+

.. note::
    | 1. Spark time is the execution time of funciton "pageRank.runUntilConvergence".
    | 2. Spark running on platform with Intel(R) Xeon(R) CPU E5-2690 v4 @2.600GHz, 56 Threads (2 Sockets, 14 Core(s) per socket, 2 Thread(s) per core).
    | 3. time unit: second.
    | 4. "-" Indicates that the result could not be obtained due to insufficient memory.
    | 5. FPGA time is the kernel runtime by adding data transfer and executed with pagerank_cache
    | 6. Collected on Alveo u50 platform

.. toctree::
   :maxdepth: 1


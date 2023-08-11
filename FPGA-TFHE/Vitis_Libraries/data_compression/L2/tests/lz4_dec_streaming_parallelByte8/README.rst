==================================
Xlinx LZ4 Streaming Decompression 
==================================

LZ4 Compress Streaming example resides in ``L2/tests/lz4_dec_streaming_parallelByte8`` directory. 

Follow build instructions to generate host executable and binary.

The binary host file generated is named as **lz4** and it is present in ``./build`` directory.

Executable Usage
----------------

1. To execute single file for decompression             : ``./build/xil_lz4_streaming -xbin ./build/xclbin_<xsa_name>_<TARGET mode>/decompress_streaming.xclbin -d <input file_name>``
2. To execute multiple files for decompression    : ``./build/xil_lz4_streaming -xbin ./build/xclbin_<xsa_name>_<TARGET mode>/decompress_streaming.xclbin -dfl <files.list>``

    - ``<files.list>``: Contains various file names with current path

The usage of the generated executable is as follows:

.. code-block:: bash
       
   Usage: application.exe -[-h-d-dfl-xbin-id]
          --help,                -h        Print Help Options
          --decompress,          -d        Decompress
          --decompress_list,     -dfl      Decompress List of compressed Input Files
          --max_cr,              -mcr      Maximum CR                                            Default: [10]
          --xclbin,              -xbin     XCLBIN
          --device_id,           -id       Device ID                                             Default: [0]
          --block_size,          -B        Compress Block Size [0-64: 1-256: 2-1024: 3-4096]     Default: [0]

Resource Utilization 
~~~~~~~~~~~~~~~~~~~~~

Table below presents resource utilization of Xilinx LZ4 Streaming Compression kernels. 
The final Fmax achieved is 300MHz                                                                                                                   

========== ===== ====== ===== ===== ===== 
Flow       LUT   LUTMem REG   BRAM  URAM 
========== ===== ====== ===== ===== ===== 

Decompress 5.3K  750    4.8K   0     2

========== ===== ====== ===== ===== ===== 

Performance Data
~~~~~~~~~~~~~~~~

Table below presents kernel throughput achieved for a single compute
unit. 

============================= =========================
Topic                         Results
============================= =========================
Decompression Throughput       1.8 GB/s
============================= =========================

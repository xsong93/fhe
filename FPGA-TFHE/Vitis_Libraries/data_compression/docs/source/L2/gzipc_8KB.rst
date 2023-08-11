=====================================
Xilinx GZip Streaming 8KB Compression
=====================================

GZip example resides in ``L2/tests/gzipc_8KB`` directory. 

Follow build instructions to generate host executable and binary.

The binary host file generated is named as "**xil_gzip**" and it is present in ``./build`` directory.

Executable Usage
----------------

1. To execute single file for compression 	    : ``./build/xil_gzip -xbin ./build/xclbin_<xsa_name>_<TARGET mode>/compress.xclbin -c <file_name>``
2. To execute multiple files for compression    : ``./build/xil_gzip -xbin ./build/xclbin_<xsa_name>_<TARGET mode>/compress.xclbin -cfl <files.list>``

	- ``<files.list>``: Contains various file names with current path

The usage of the generated executable is as follows:

.. code-block:: bash
 
   Usage: application.exe -[-h-c-l-xbin-B]
          --help,           -h      Print Help Options
          --xclbin,         -xbin   XCLBIN                                               Default: [compress]
          --compress,       -c      Compress
          --file_list,      -cfl    Compress List of Input Files
          --max_cr,         -mcr    Maximum CR    
          --device_id,      -id     Device ID                                       Default: [0]
          --zlib,           -zlib   [0:GZip, 1:Zlib]                                Default: [0]

Results
-------

Resource Utilization 
~~~~~~~~~~~~~~~~~~~~~

Table below presents resource utilization of Xilinx GZip Compress/Decompress
kernels. The final Fmax achieved is 300MHz.


========== ===== ====== ===== ===== ===== 
Flow       LUT   LUTMem REG   BRAM  URAM 
========== ===== ====== ===== ===== ===== 
Compress   57.5K 8.6K   52.5K 100   48    
========== ===== ====== ===== ===== ===== 

Performance Data
~~~~~~~~~~~~~~~~

Table below presents kernel throughput achieved for a single compute
unit. 

============================= =========================
Topic                         Results
============================= =========================
Compression Throughput        2 GB/s
Average Compression Ratio     2.5x (Silesia Benchmark)
============================= =========================

Standard GZip Support
---------------------

This application is compatible with standard Gzip/Zlib application (compress/decompress).  

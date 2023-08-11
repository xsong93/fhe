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

#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <algorithm>

#include "xcl2.hpp"
#include "xf_utils_sw/logger.hpp"

#include "matrixUtility.hpp"

// Memory alignment
template <typename T>
T* aligned_alloc(std::size_t num) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 4096, num * sizeof(T))) {
        throw std::bad_alloc();
    }
    return reinterpret_cast<T*>(ptr);
}

// Compute time difference
unsigned long diff(const struct timeval* newTime, const struct timeval* oldTime) {
    return (newTime->tv_sec - oldTime->tv_sec) * 1000000 + (newTime->tv_usec - oldTime->tv_usec);
}

// Arguments parser
class ArgParser {
   public:
    ArgParser(int& argc, const char** argv) {
        for (int i = 1; i < argc; ++i) mTokens.push_back(std::string(argv[i]));
    }
    bool getCmdOption(const std::string option, std::string& value) const {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
        if (itr != this->mTokens.end() && ++itr != this->mTokens.end()) {
            value = *itr;
            return true;
        }
        return false;
    }

   private:
    std::vector<std::string> mTokens;
};

//! Core function of QR benchmark
int main(int argc, const char* argv[]) {
    // Initialize parser
    ArgParser parser(argc, argv);

    // Initialize paths addresses
    std::string xclbin_path;
    std::string num_str;
    int num_runs, numRow, numCol, seed;

    // Read In paths addresses
    if (!parser.getCmdOption("-xclbin", xclbin_path)) {
        std::cout << "INFO:input path is not set!\n";
    }
    if (!parser.getCmdOption("-runs", num_str)) {
        num_runs = 1;
        std::cout << "INFO:row size M is not set!\n";
    } else {
        num_runs = std::stoi(num_str);
    }
    if (!parser.getCmdOption("-M", num_str)) {
        numRow = 16;
        std::cout << "INFO:row size M is not set!\n";
    } else {
        numRow = std::stoi(num_str);
    }
    if (!parser.getCmdOption("-N", num_str)) {
        numCol = 16;
        std::cout << "INFO:column size N is not set!\n";
    } else {
        numCol = std::stoi(num_str);
    }
    if (!parser.getCmdOption("-seed", num_str)) {
        seed = 12;
        std::cout << "INFO:seed is not set!\n";
    } else {
        seed = std::stoi(num_str);
    }

    // Platform related operations
    xf::common::utils_sw::Logger logger;
    cl_int err = CL_SUCCESS;

    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &err);
    logger.logCreateContext(err);

    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    logger.logCreateCommandQueue(err);

    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("INFO: Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbin_path);
    devices.resize(1);

    cl::Program program(context, devices, xclBins, NULL, &err);
    logger.logCreateProgram(err);

    cl::Kernel kernel_geqrf_0(program, "kernel_geqrf_0", &err);
    logger.logCreateKernel(err);

    // Output the inputs information
    std::cout << "INFO: Matrix Row M: " << numRow << std::endl;
    std::cout << "INFO: Matrix Col N: " << numCol << std::endl;

    // Initialization of host buffers
    int out_size_tau = numRow;
    int in_size = numRow * numCol;

    double* dataA_qrd;
    double* tau_qrd;
    dataA_qrd = aligned_alloc<double>(in_size);
    tau_qrd = aligned_alloc<double>(out_size_tau);

    // Generate general matrix numRow x numCol
    matGen<double>(numRow, numCol, seed, dataA_qrd);

    double* dataA = new double[in_size];
    for (int i = 0; i < in_size; ++i) {
        dataA[i] = dataA_qrd[i];
    }

    // DDR Settings
    std::vector<cl_mem_ext_ptr_t> mext_i(1);
    std::vector<cl_mem_ext_ptr_t> mext_o(1);
    // mext_i[0].flags = XCL_MEM_DDR_BANK0;
    // mext_o[0].flags = XCL_MEM_DDR_BANK0;
    // mext_i[0].obj = dataA_qrd;
    // mext_i[0].param = 0;
    // mext_o[0].obj = tau_qrd;
    // mext_o[0].param = 0;
    mext_i[0] = {0, dataA_qrd, kernel_geqrf_0()};
    mext_o[0] = {1, tau_qrd, kernel_geqrf_0()};

    // Create device buffer and map dev buf to host buf
    std::vector<cl::Buffer> input_buffer(1), output_buffer(1);

    input_buffer[0] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                 sizeof(double) * in_size, &mext_i[0]);
    output_buffer[0] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                  sizeof(double) * out_size_tau, &mext_o[0]);

    // Data transfer from host buffer to device buffer
    std::vector<std::vector<cl::Event> > kernel_evt(2);
    kernel_evt[0].resize(1);
    kernel_evt[1].resize(1);

    std::vector<cl::Memory> ob_in, ob_out;
    ob_in.push_back(input_buffer[0]);
    ob_out.push_back(input_buffer[0]);
    ob_out.push_back(output_buffer[0]);

    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &kernel_evt[0][0]); // 0 : migrate from host to dev
    q.finish();
    std::cout << "INFO: Finish data transfer from host to device" << std::endl;

    // Setup kernel
    kernel_geqrf_0.setArg(0, input_buffer[0]);
    kernel_geqrf_0.setArg(1, output_buffer[0]);
    q.finish();
    std::cout << "INFO: Finish kernel setup" << std::endl;

    // Variables to measure time
    struct timeval tstart, tend;

    // Launch kernel and compute kernel execution time
    gettimeofday(&tstart, 0);

    q.enqueueTask(kernel_geqrf_0, nullptr, nullptr);

    q.finish();
    gettimeofday(&tend, 0);
    std::cout << "INFO: Finish kernel execution" << std::endl;
    int exec_time = diff(&tend, &tstart);
    std::cout << "INFO: FPGA executiom per run: " << exec_time << " us\n";

    // Data transfer from device buffer to host buffer
    q.enqueueMigrateMemObjects(ob_out, 1, nullptr, nullptr); // 1 : migrate from dev to host
    q.finish();

    // Calculate A_out = Q*R and compare with original A matrix
    double* Q = new double[numRow * numRow];
    constructQ<double>(dataA_qrd, tau_qrd, numRow, numCol, Q);

    convertToRInline<double>(dataA_qrd, numRow, numCol);

    double* A = new double[numRow * numCol];
    matrixMult<double>(Q, numRow, numRow, dataA_qrd, numRow, numCol, A);

    bool equal = compareMatrices<double>(A, dataA, numRow, numCol, numCol);

    if (equal) {
        logger.info(xf::common::utils_sw::Logger::Message::TEST_PASS);
    } else {
        logger.error(xf::common::utils_sw::Logger::Message::TEST_FAIL);
    }

    // Delete created buffers
    delete[] A;
    delete[] Q;

    delete[] dataA;
}

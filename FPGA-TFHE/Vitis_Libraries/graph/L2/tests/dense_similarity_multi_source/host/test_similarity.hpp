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

#ifndef XF_GRAPH_TEST_SIMILARITY_HPP
#define XF_GRAPH_TEST_SIMILARITY_HPP

#ifndef HLS_TEST
#include "xcl2.hpp"
#endif

#include "utils.hpp"
#include "xf_utils_sw/logger.hpp"

template <int PUNUM>
int computeSimilarity(std::string xclbinPath,
                      std::string goldenFile,
                      unsigned int numVertices,
                      unsigned int numEdges,
                      int similarityType,
                      int dataType,
                      int batchNUM,
                      int sortK,
                      int repInt,
                      unsigned int numVerticesPU[PUNUM],
                      unsigned int numEdgesPU[PUNUM],
                      float* weightDense[4 * PUNUM],
                      float* sourceWeight) {
    struct timeval start_time; // End to end time clock start
    gettimeofday(&start_time, 0);

    // output && config////////////////////////////////////////////////////////////////
    std::vector<ap_int<32>*> config(repInt);
    std::vector<ap_int<32>*> result_id(repInt);
    std::vector<float*> similarity(repInt);
    unsigned int startID[PUNUM];
    unsigned int tmp = 0;
    for (int i = 0; i < PUNUM - 1; i++) { // calculate multi PU start address
        startID[i] = tmp;
        tmp += 4 * numVerticesPU[i];
    }
    startID[PUNUM - 1] = tmp;
    for (int i = 0; i < repInt; i++) {
        similarity[i] = aligned_alloc<float>(65536);
        result_id[i] = aligned_alloc<ap_int<32> >(65536);
        config[i] = aligned_alloc<ap_int<32> >(64);
        config[i][0] = sortK;
        config[i][1] = numEdges;
        config[i][2] = similarityType;
        config[i][3] = dataType;
        config[i][4] = startID[0];
        config[i][5] = numVerticesPU[0];
        config[i][6] = numEdgesPU[0];
        config[i][7] = batchNUM;
    }

#ifndef HLS_TEST
    xf::common::utils_sw::Logger logger(std::cout, std::cerr);
    cl_int fail;

    // platform related operations
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Creating Context and Command Queue for selected Device
    cl::Context context(device, NULL, NULL, NULL, &fail);
    logger.logCreateContext(fail);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &fail);
    logger.logCreateCommandQueue(fail);
    std::string devName = device.getInfo<CL_DEVICE_NAME>();
    printf("INFO: Found Device=%s\n", devName.c_str());

    cl::Program::Binaries xclBins = xcl::import_binary_file(xclbinPath);
    devices.resize(1);
    cl::Program program(context, devices, xclBins, NULL, &fail);
    logger.logCreateProgram(fail);

    // create kernels
    std::vector<cl::Kernel> similarity_kernel(repInt);
    for (int i = 0; i < repInt; i++) {
        similarity_kernel[i] = cl::Kernel(program, "denseSimilarityKernel", &fail);
        logger.logCreateKernel(fail);
    }
    std::cout << "INFO: kernel has been created" << std::endl;

    // declare map of host buffers
    std::vector<cl_mem_ext_ptr_t> mext_o(3 * repInt + 4 * PUNUM + 1);
    for (int i = 0; i < PUNUM; i++) {
        mext_o[4 * i + 0] = {2, weightDense[4 * i], similarity_kernel[0]()};
        mext_o[4 * i + 1] = {3, weightDense[4 * i + 1], similarity_kernel[0]()};
        mext_o[4 * i + 2] = {4, weightDense[4 * i + 2], similarity_kernel[0]()};
        mext_o[4 * i + 3] = {5, weightDense[4 * i + 3], similarity_kernel[0]()};
    }

    mext_o[4 * PUNUM] = {1, sourceWeight, similarity_kernel[0]()};

    for (int i = 0; i < repInt; i++) {
        mext_o[4 * PUNUM + 1 + i] = {0, config[i], similarity_kernel[0]()};
        mext_o[4 * PUNUM + 1 + repInt + i] = {6, result_id[i], similarity_kernel[0]()};
        mext_o[4 * PUNUM + 1 + 2 * repInt + i] = {7, similarity[i], similarity_kernel[0]()};
    }

    // create device buffer and map dev buf to host buf
    cl::Buffer weight_buf[4 * PUNUM];
    cl::Buffer source_weight_buf;
    std::vector<cl::Buffer> config_buf(repInt);
    std::vector<cl::Buffer> result_id_buf(repInt);
    std::vector<cl::Buffer> similarity_buf(repInt);

    // declare cl::buffers
    for (int i = 0; i < 4 * PUNUM; i++) {
        int sizeW = numVerticesPU[i / 4] * numEdges;
        weight_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   sizeof(ap_int<32>) * (sizeW + CHANNEL_NUMBER), &mext_o[i]);
    }
    source_weight_buf = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   sizeof(ap_int<32>) * (numEdges * batchNUM + CHANNEL_NUMBER), &mext_o[4 * PUNUM]);
    for (int i = 0; i < repInt; i++) {
        config_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                   sizeof(ap_int<32>) * 64, &mext_o[4 * PUNUM + 1 + i]);
        result_id_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                      sizeof(ap_int<32>) * 65536, &mext_o[4 * PUNUM + 1 + repInt + i]);
        similarity_buf[i] = cl::Buffer(context, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                       sizeof(float) * 65536, &mext_o[4 * PUNUM + 1 + 2 * repInt + i]);
    }

    // add buffers to migrate
    std::vector<cl::Memory> init;
    for (int i = 0; i < repInt; i++) {
        init.push_back(config_buf[i]);
    }
    for (int i = 0; i < 4 * PUNUM; i++) {
        init.push_back(weight_buf[i]);
    }
    for (int i = 0; i < repInt; i++) {
        init.push_back(result_id_buf[i]);
    }
    for (int i = 0; i < repInt; i++) {
        init.push_back(similarity_buf[i]);
    }
    init.push_back(source_weight_buf);

    // migrate data from host to device
    q.enqueueMigrateMemObjects(init, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, nullptr, nullptr);
    q.finish();
    std::cout << "INFO: buffer initialized" << std::endl;

    std::vector<cl::Memory> ob_in;
    std::vector<cl::Memory> ob_out;

    for (int i = 0; i < repInt; i++) {
        ob_in.push_back(config_buf[i]);
    }
    for (int i = 0; i < 4 * PUNUM; i++) {
        ob_in.push_back(weight_buf[i]);
    }
    ob_in.push_back(source_weight_buf);

    for (int i = 0; i < repInt; i++) {
        ob_out.push_back(result_id_buf[i]);
        ob_out.push_back(similarity_buf[i]);
    }

    // declare events
    std::vector<cl::Event> events_write(1);
    std::vector<std::vector<cl::Event> > events_kernel(repInt);
    std::vector<cl::Event> events_read(1);
    for (int i = 0; i < repInt; ++i) {
        events_kernel[i].resize(1);
    }

    // set kernel args
    for (int i = 0; i < repInt; i++) {
        int j = 0;
        similarity_kernel[i].setArg(j++, config_buf[i]);
        similarity_kernel[i].setArg(j++, source_weight_buf);

        for (int k = 0; k < 4 * PUNUM; k++) {
            similarity_kernel[i].setArg(j++, weight_buf[k]);
        }

        similarity_kernel[i].setArg(j++, result_id_buf[i]);
        similarity_kernel[i].setArg(j++, similarity_buf[i]);
    }

    // launch kernel and calculate kernel execution time
    std::cout << "INFO: Kernel Start" << std::endl;

    // migrate data from host to device
    q.enqueueMigrateMemObjects(ob_in, 0, nullptr, &events_write[0]);
    q.finish();

    // kernel execution
    q.enqueueTask(similarity_kernel[0], &events_write, &events_kernel[0][0]);
    for (int i = 1; i < repInt; i++) {
        q.enqueueTask(similarity_kernel[i], &events_kernel[i - 1], &events_kernel[i][0]);
    }

    // migrate data from device to host
    q.enqueueMigrateMemObjects(ob_out, 1, &events_kernel[repInt - 1], &events_read[0]);
    q.finish();

    struct timeval end_time;
    gettimeofday(&end_time, 0);
    std::cout << "INFO: Finish kernel execution" << std::endl;
    std::cout << "INFO: Finish E2E execution" << std::endl;

    // print related times
    unsigned long timeStart, timeEnd, exec_time0;
    std::cout << "-------------------------------------------------------" << std::endl;
    events_write[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
    events_write[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
    exec_time0 = (timeEnd - timeStart) / 1000.0;
    logger.info(xf::common::utils_sw::Logger::Message::TIME_H2D_MS, exec_time0);
    std::cout << "-------------------------------------------------------" << std::endl;
    events_read[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
    events_read[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
    exec_time0 = (timeEnd - timeStart) / 1000.0;
    logger.info(xf::common::utils_sw::Logger::Message::TIME_D2H_MS, exec_time0);
    std::cout << "-------------------------------------------------------" << std::endl;
    exec_time0 = 0;
    for (int i = 0; i < repInt; ++i) {
        events_kernel[i][0].getProfilingInfo(CL_PROFILING_COMMAND_START, &timeStart);
        events_kernel[i][0].getProfilingInfo(CL_PROFILING_COMMAND_END, &timeEnd);
        exec_time0 += (timeEnd - timeStart) / 1000.0;
    }
    logger.info(xf::common::utils_sw::Logger::Message::TIME_KERNEL_MS, exec_time0 / repInt);
    std::cout << "-------------------------------------------------------" << std::endl;
    unsigned long exec_timeE2E = diff(&end_time, &start_time);
    std::cout << "INFO: FPGA execution time of " << repInt << " runs:" << exec_timeE2E << " us\n"
              << "INFO: Average execution per run: " << exec_timeE2E - exec_time0 * repInt + exec_time0 << " us\n";
    std::cout << "-------------------------------------------------------" << std::endl;

#else
    denseSimilarityKernel(config[0], sourceWeight, weightDense[0], weightDense[1], weightDense[2], weightDense[3],
                          result_id[0], similarity[0]);
#endif

    // need to write a compare function in order to compare golden values with results and put it here
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    int err = 0;
    for (int i = 0; i < batchNUM; i++)
        err |= checkData<PU_NUMBER>(goldenFile, result_id[0] + sortK * i, similarity[0] + sortK * i);

    if (err) {
        logger.error(xf::common::utils_sw::Logger::Message::TEST_FAIL);
    } else {
        logger.info(xf::common::utils_sw::Logger::Message::TEST_PASS);
    }
    return err;
}

#endif //#ifndef VT_GRAPH_SIMILARITY_H

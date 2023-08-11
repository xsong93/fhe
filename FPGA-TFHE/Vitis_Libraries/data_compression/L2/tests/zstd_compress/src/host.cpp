/*
 * (c) Copyright 2020 Xilinx, Inc. All rights reserved.
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
 *
 */
#include "zstdApp.hpp"
#include "zstdOCLHost.hpp"
#include <memory>

int main(int argc, char* argv[]) {
    bool enable_profile = false;
    uint16_t test_itr_cnt = 1;
    compressBase::State flow = compressBase::COMPRESS;
    compressBase::Level lflow = compressBase::SEQ;

    // Driver class object
    zstdApp d(argc, argv, lflow);

    // Design class object creating and constructor invocation
    std::unique_ptr<zstdOCLHost> zstd(new zstdOCLHost(flow, d.getXclbin(), d.getDeviceId(), test_itr_cnt));

    // Run API to launch the compress or decompress engine
    d.run(zstd.get(), enable_profile);
    return 0;
}

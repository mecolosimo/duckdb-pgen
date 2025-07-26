#pragma once

#include <cstdint>

#include "duckdb/common/helper.hpp"
#include "duckdb/common/multi_file/base_file_reader.hpp"
#include "utf8proc_wrapper.hpp"
#include "duckdb/storage/caching_file_system.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/common/multi_file/multi_file_data.hpp"

#include "pgen_reader.hpp"

namespace duckdb {

typedef unsigned char AlleleCode;

struct PgenVariantStruct {
        std::uintptr_t* genovec;
        std::uintptr_t* patch_01_set;
        AlleleCode* patch_01_vals;
        std::uintptr_t* patch_10_set;
        AlleleCode* patch_10_vals;
        std::uintptr_t* phasepresent;
        std::uintptr_t* phaseinfo;
        std::uintptr_t* dosage_present;
        std::uint16_t* dosage_main;
        std::uint32_t patch_01_ct;
        std::uint32_t patch_10_ct;
        std::uint32_t phasepresent_ct;
};

static void ReadPgenFunction(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
    // TODO
    output.SetCardinality(0);
} 

} // namespace duckdb
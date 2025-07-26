#pragma once

#include <sstream>
#include <string>

#include "pgen_reader.hpp"
#include "pgen_utils.hpp"

// Biggest thing to watch out for is mixing of Halfword with these constants in
// 32-bit builds.  Dosage and Vec8thUint are also relevant.)
#ifdef __cplusplus
#  define CONSTI32(name, expr) const int32_t name = (expr)
#else
#  define CONSTI32(name, expr) enum {name = (expr)}
#endif

namespace duckdb {

// maximum prime < 2^32 is 4294967291; quadratic hashing guarantee breaks down
// past that divided by 2.
CONSTI32(kPglMaxVariantCt, 0x7ffffffd);

CONSTI32(kPglMaxSampleCt, 0x7ffffffe);
class PgenExtension : public Extension {
public:
	void Load(DuckDB &db) override;
	std::string Name() override;
    std::string Version() const override;
};

struct ReadPgenBindData : public TableFunctionData {
	//std::vector<std::string> selected_fields;
	std::vector<duckdb::LogicalType> field_types;

	// Header stuff
	uint8_t	 storage_mode;
	uint32_t raw_variant_ct; // cur_sample_ct from file modified? by PgfiInitPhase1
	uint32_t raw_sample_ct;
	uint8_t  header_ctrl;
	//uint32_t max_allele_ct;
    //unsigned char* vrtypes;
    //uintptr_t* allele_idx_offsets;
    //uintptr_t* nonref_flags;
    //uint32_t gflags;
	
	// Reader stuff
	std::string file_path;
	bool finished = false;  	// Track if finished processing
	bool variant_block = false;	// Currently in a variant block
	uint32_t current_variant = 0;
	uint32_t current_sample = 0;
};

} // namespace duckdb

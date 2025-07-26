#define DUCKDB_EXTENSION_MAIN

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/helper.hpp"
#include "duckdb/common/multi_file/base_file_reader.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include "pgen_extension.hpp"
namespace duckdb {


//! This is where the actual binding must happen, so in this function we
//! 1. Open the file
//! 2. Check if a pgen
//! 3. Set header stuff and reader setup
static unique_ptr<FunctionData> ReadPgenBind(ClientContext &context,
											 TableFunctionBindInput &input,
											 vector<LogicalType> &return_types,
											 vector<string> &names) {
	auto bind_data = make_uniq<ReadPgenBindData>();

    // Extract the file path argument
    auto &fs = FileSystem::GetFileSystem(context);
    string file_path = input.inputs[0].GetValue<string>();

	// **Open remote or local file using DuckDB’s FileSystem API**
    std::unique_ptr<FileHandle> file_handle = fs.OpenFile(file_path, FileFlags::FILE_FLAGS_READ);
    if (!file_handle) {
        throw IOException("Failed to open file: " + file_path);
    }

	CachingFileSystem caching_file_system = CachingFileSystem::Get(context);

	// See https://github.com/chrchang/plink-ng/blob/9e2ebe5d5197f0e84f2a98f1a8c4b164675a4809/2.0/include/pgenlib_read.cc
	auto caching_file_handle = caching_file_system.OpenFile(file_path, FileOpenFlags::FILE_FLAGS_READ);
    uint32_t magic = read_uint_16_le(caching_file_handle);

    if (magic != 0x1B6C) {
		std::stringstream ss;
		ss << "Error: " << file_path << " not a pgen file";
		ss << std::hex << std::uppercase << magic << std::nouppercase << std::dec << std::endl;
        throw IOException(ss.str());
    }

	// Check size (if size < 4) 
	idx_t fsize = caching_file_handle->GetFileSize();

	if (fsize < 4) {
		throw IOException("Error: " + file_path + " is too small to be a valid .pgen file.");
	}

	uint8_t file_type_code = read_uint8(caching_file_handle);
	
	if ((file_type_code & 0xfe) != 0x20) {
		if ((file_type_code & 0xfe) == 0x30) {
			// how did we get here?
			throw IOException(file_path + " is a .pgen.pgi index file, rather than a .pgen file.");
		}
	}

	if (file_type_code != 0x10) {
		// Is there things between 0x20 and 0x30, (file_type_code & 0xfe) ?
		std::stringstream ss;
		ss << file_path << " in an unsupported pgen type (storage mode); ";
		ss << std::to_string(file_type_code) << std::endl;
		throw IOException(ss.str());
	}

	if (fsize < 12) {
		throw IOException(file_path + " is too small to be a valid .pgen file.");
	}

	// Reader file details
    bind_data->raw_variant_ct = read_uint_32_le(caching_file_handle);
	
	if (((bind_data->raw_variant_ct - 1) > (kPglMaxVariantCt - 1)) && (bind_data->raw_variant_ct != UINT32_MAX)) {
		throw IOException("Error: Invalid variant count in pgen");
	}

	if ((bind_data->raw_variant_ct - 1) > (kPglMaxVariantCt - 1)) {
		throw IOException("Error: Invalid variant count in .pgen file.");
	}

    bind_data->raw_sample_ct = read_uint_32_le(caching_file_handle);
	if (((bind_data->raw_sample_ct - 1) > (kPglMaxSampleCt - 1)) && (bind_data->raw_sample_ct != UINT32_MAX)) {
		throw IOException("Error: Invalid sample count in pgen");
	}

	if ((bind_data->raw_sample_ct - 1) > (kPglMaxSampleCt - 1)) {
		throw IOException("Error: Invalid sample count in .pgen file.");
	}

	// header_ctrl (header control), twelfth byte
	// 	Bits 0-3 indicate how variant-record types and lengths are stored
	// 	Bits 4-5 indicate how many bytes are used to store each allele count. Zero indicates that no allele-count information is stored
	//	Bits 6-7 indicate how “provisional reference” flag information is stored. 
	//		0 indicates that this information isn’t in the PGEN (and should be present in the accompanying PVAR)
	//		1 indicates that no reference alleles are provisional (all are trusted since they were e.g. imported from a VCF)
	//		2 indicates that all reference alleles are provisional 
	//			(none are trusted, usually because the PGEN was created from a PLINK 1 fileset which isn’t
	//			designed to track REF/ALT alleles)
	//		3 indicates that some but not all reference alleles are provisional, and this is tracked by bitarrays in the header.
	uint8_t header_ctrl = read_uint8(caching_file_handle);

	//  pgfip->gflags = kfPgenGlobal0;

	// explicit storage of "is this reference allele untrusted?"
	uint32_t nonref_flags_storage = header_ctrl >> 6;
	//if (nonref_flags_storage == 2) {
	//  pgfip->gflags |= kfPgenGlobalAllNonref;
	//}

	// Construct return columns
	names.push_back({"variant column"});
	return_types.push_back({LogicalTypeId::INTEGER});
	names.push_back({"sample column"});
	return_types.push_back({LogicalTypeId::INTEGER});
	names.push_back({"allele0"});
	return_types.push_back({LogicalTypeId::FLOAT});
	names.push_back({"allele1"});
	return_types.push_back({LogicalTypeId::FLOAT});
	names.push_back({"phased"});
	return_types.push_back({LogicalTypeId::BOOLEAN});	// how to specify default?

	return std::move(bind_data);
}

static void LoadInternal(DatabaseInstance &instance) {
	// Register a read_pgen function
	auto read_pgen_selected = TableFunction("read_pgen",
	{ LogicalType::VARCHAR, },  ReadPgenFunction, ReadPgenBind);
	//read_pgan_selected.named_parameters["phased"] = LogicalType::BOOL;

    ExtensionUtil::RegisterFunction(instance, read_pgen_selected);
}

void PgenExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string PgenExtension::Name() {
	return "PLINK2";
}

std::string PgenExtension::Version() const {
#ifdef EXT_VERSION_PGEN
	return EXT_VERSION_PGEN;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void pgen_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::PgenExtension>();
}

DUCKDB_EXTENSION_API const char *pgen_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif

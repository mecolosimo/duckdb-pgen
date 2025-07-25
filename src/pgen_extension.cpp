#define DUCKDB_EXTENSION_MAIN

#include "pgen_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

// OpenSSL linked through vcpkg
#include <openssl/opensslv.h>

namespace duckdb {

inline void PgenScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Pgen " + name.GetString() + " 🐥");
	});
}

inline void PgenOpenSSLVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Pgen " + name.GetString() + ", my linked OpenSSL version is " +
		                                           OPENSSL_VERSION_TEXT);
	});
}

static void LoadInternal(DatabaseInstance &instance) {
	// Register a scalar function
	auto pgen_scalar_function = ScalarFunction("pgen", {LogicalType::VARCHAR}, LogicalType::VARCHAR, PgenScalarFun);
	ExtensionUtil::RegisterFunction(instance, pgen_scalar_function);

	// Register another scalar function
	auto pgen_openssl_version_scalar_function = ScalarFunction("pgen_openssl_version", {LogicalType::VARCHAR},
	                                                            LogicalType::VARCHAR, PgenOpenSSLVersionScalarFun);
	ExtensionUtil::RegisterFunction(instance, pgen_openssl_version_scalar_function);
}

void PgenExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string PgenExtension::Name() {
	return "pgen";
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

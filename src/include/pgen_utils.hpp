#pragma once

#include <fstream>
#include <vector>
#include <cstdint>

#include "duckdb/storage/caching_file_system.hpp"
#include "duckdb/storage/buffer/buffer_handle.hpp"

namespace duckdb {

//! Read an uint32 in little endian order (4 bytes) from a file
inline uint32_t read_uint_32_le(unique_ptr<duckdb::CachingFileHandle>& caching_file_handle) {
	duckdb::data_ptr_t bytes = nullptr;
    duckdb::idx_t size = 4;
    auto buf_handle = caching_file_handle->Read(bytes, size); // BufferHandle
	auto buffer_data = buf_handle.Ptr();

	D_ASSERT(buf_handle.IsValid());
	D_ASSERT(buffer_data == bytes);

    uint32_t result = (uint32_t(bytes[3]) << 24) |
                      (uint32_t(bytes[2]) << 16) |
                      (uint32_t(bytes[1]) << 8)  |
                      (uint32_t(bytes[0]));
    return result;

    /*
    #include <bit>

    if constexpr (std::endian::native == std::endian::big) {
 
    } else if constexpr (std::endian::native == std::endian::little) {

    } else {} */
}

//! Read an uint16 in little endian order (2 bytes) from a file into uint32 (top will be 0)
inline uint32_t read_uint_16_le(unique_ptr<duckdb::CachingFileHandle>& caching_file_handle) {
	duckdb::data_ptr_t bytes = nullptr;
    duckdb::idx_t size = 2;
    auto buf_handle = caching_file_handle->Read(bytes, size); // BufferHandle
	auto buffer_data = buf_handle.Ptr();

	D_ASSERT(buf_handle.IsValid());
	D_ASSERT(buffer_data == bytes);

    uint32_t result = 0 | (uint32_t(bytes[1]) << 8)  |
                      (uint32_t(bytes[0]));
    return result;
}

//! Read an uint32 in big endian order (4 bytes) from a file
inline uint32_t read_uint_32_be(unique_ptr<duckdb::CachingFileHandle>& caching_file_handle) {
	duckdb::data_ptr_t bytes = nullptr;
    duckdb::idx_t size = 4;
    auto buf_handle = caching_file_handle->Read(bytes, size);
	auto buffer_data = buf_handle.Ptr();

	D_ASSERT(buf_handle.IsValid());
	D_ASSERT(buffer_data == bytes);

    uint32_t result = (uint32_t(bytes[0]) << 24) |
                      (uint32_t(bytes[1]) << 16) |
                      (uint32_t(bytes[2]) << 8)  |
                      (uint32_t(bytes[3]));
    return result;
}

//! Read an byte (uint8) from a file
inline uint8_t read_uint8(unique_ptr<duckdb::CachingFileHandle>& caching_file_handle) {
	duckdb::data_ptr_t bytes = nullptr;
    duckdb::idx_t size = 1;
    auto buf_handle = caching_file_handle->Read(bytes, size);
	auto buffer_data = buf_handle.Ptr();

	D_ASSERT(buf_handle.IsValid());
	D_ASSERT(buffer_data == bytes);

    uint8_t result = uint8_t(bytes[0]);
    return result;
}

} // namespace duckdb
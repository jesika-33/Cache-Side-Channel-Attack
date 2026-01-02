#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <string>

namespace CACHE{

/// @brief User-defined literal for byte size
/// @param size 
/// @return size in bytes
constexpr uint32_t operator""_Bytes(unsigned long long size) {
	return static_cast<uint32_t>(size);
}
/// @brief User-defined literal for kilobyte size
/// @param size 
/// @return size in bytes
constexpr uint32_t operator""_KiB(unsigned long long size) {
	return static_cast<uint32_t>(size << 10);
}
/// @brief User-defined literal for megabyte size
/// @param size 
/// @return size in bytes
constexpr uint32_t operator""_MiB(unsigned long long size) {
	return static_cast<uint32_t>(size << 20);
}

/// @brief Cache simulator class
class Cache {
private:
	const uint32_t hit_latency = 1u;
	const uint32_t miss_latency = 100u;
	const uint32_t writethrough_latency = 20u;
	uint32_t _block_size;
	uint32_t _associativity;
	uint32_t _set_count;
	std::string _replacement_policy;
	bool _write_back;
	bool _write_allocate;
	uint32_t _read_count;
	uint32_t _read_limit;
	uint32_t _write_count;
	uint32_t _write_limit;

	bool **valid;
	bool **dirty;
	uint32_t **tag;
	uint32_t **cnt;

	/// @brief Get the index of the cache line for a given address
	/// @param address 
	/// @return index of the cache line
	uint32_t _index(uint32_t address);
	/// @brief Get the offset within a cache block for a given address
	/// @param address 
	/// @return offset within the cache block
	uint32_t _offset(uint32_t address);
	/// @brief Get the tag for a given address
	/// @param address
	/// @return tag for the cache line
	uint32_t _tag(uint32_t address);

	/// @brief Increment the LFU counter for a given set and way
	/// @param set_index
	/// @param way
	void _update_lfu(uint32_t set_index, uint32_t way);
	/// @brief Query the way with the lowest LFU counter in a given set
	/// @param set_index 
	/// @return way with the lowest LFU counter
	uint32_t _query_lfu(uint32_t set_index);
	/// @brief Student-implemented LRU update function
	/// @param set_index
	/// @param way
	void _update_lru_student_implementation(uint32_t set_index, uint32_t way);
	/// @brief Student-implemented LRU query function
	/// @param set_index
	/// @return way with the highest LRU counter
	uint32_t _query_lru_student_implementation(uint32_t set_index);

	/// @brief Query an empty way in a given set
	/// @param set_index
	/// @return empty way, or _associativity if not found
	uint32_t _query_empty(uint32_t set_index);
	/// @brief Query a way with a given tag in a given set
	/// @param set_index
	/// @param tag
	/// @return way with the given tag, or _associativity if not found
	uint32_t _query_tag(uint32_t set_index, uint32_t tag);
	/// @brief Evict a cache line in a given set based on the replacement policy
	/// @param set_index
	/// @return way to be evicted
	uint32_t _evict(uint32_t set_index);
	/// @brief Read the cache with a given address. Fails if access limit exceeded.
	/// @param address
	/// @return 1 if hit, 0 if miss
	uint32_t _read(uint32_t address);
	/// @brief Write the cache with a given address. Fails if access limit exceeded.
	/// @param address
	/// @return latency of the write operation
	uint32_t _write(uint32_t address);

public:
	Cache(uint32_t block_size,
	      uint32_t associativity,
	      uint32_t set_count,
	      const std::string &replacement_policy,
		  bool write_back,
		  bool write_allocate,
		  uint32_t read_limit,
		  uint32_t write_limit);
	~Cache();
	
	/// @brief Reset the cache to its initial state with configured parameters remained
	void empty();
	/// @brief Read the cache 1024 times with given base address and stride
	/// @param base_addr starting address
	/// @param stride difference between consecutive addresses
	/// @return number of hits in 1024 accesses
	uint32_t read_1024(uint32_t base_addr, uint32_t stride);
	/// @brief Write the cache 1024 times with given base address and stride
	/// @param base_addr starting address
	/// @param stride difference between consecutive addresses
	/// @return latency of the 1024 writes
	uint32_t write_1024(uint32_t base_addr, uint32_t stride);
};

extern Cache *current_cache;

} // namespace CACHE

#endif // CACHE_H
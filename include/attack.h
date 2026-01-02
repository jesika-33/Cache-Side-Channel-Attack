#ifndef ATTACK_H
#define ATTACK_H

#include "cache.h"

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

/// @brief Student-implemented cache parameter inference attack function.
/// If the parameter is known, it is already set to the correct value;
/// otherwise, the initial value is 0 or an empty string. Students need to
/// implement the attack to infer the correct value and set the parameter.
/// @param test_case Test case number (1 to 20)
/// @param block_size Block size in bytes (16 to 3456, not necessarily power of 2)
/// @param associativity Associativity (1 to 16, power of 2)
/// @param set_count Number of sets (16 to 256, power of 2)
/// @param replacement_policy Replacement policy ("LRU" or "LFU")
/// @param write_back Whether the cache is write-back
/// @param write_allocate Whether the cache is write-allocate
void attack(uint32_t test_case,
			uint32_t& block_size,
			uint32_t& associativity,
			uint32_t& set_count,
			std::string& replacement_policy,
			bool& write_back,
			bool& write_allocate);

#endif // ATTACK_H
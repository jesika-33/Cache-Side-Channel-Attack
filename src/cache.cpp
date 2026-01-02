#include "cache.h"
#include <stdexcept>

namespace CACHE{

// public methods

Cache::Cache(uint32_t block_size,
			 uint32_t associativity,
			 uint32_t set_count,
			 const std::string &replacement_policy,
			 bool write_back,
			 bool write_allocate,
			 uint32_t read_limit,
			 uint32_t write_limit)
	: _block_size(block_size),
	  _associativity(associativity),
	  _set_count(set_count),
	  _replacement_policy(replacement_policy),
	  _write_back(write_back),
	  _write_allocate(write_allocate),
	  _read_count(0u),
	  _read_limit(read_limit),
	  _write_count(0u),
	  _write_limit(write_limit) {

	if (block_size < 4_Bytes || block_size > 512_Bytes) {
		throw std::invalid_argument("Block size must be between 4 Bytes and 512 Bytes.");
	}
	if (associativity < 1u || associativity > 16u || (associativity & (associativity - 1)) != 0) {
		throw std::invalid_argument("Associativity must be a power of 2 between 1 and 16.");
	}
	if (set_count < 1u || set_count > 256u || (set_count & (set_count - 1)) != 0) {
		throw std::invalid_argument("Set count must be a power of 2 between 1 and 256.");
	}
	if (replacement_policy != "LRU" && replacement_policy != "LFU") {
		throw std::invalid_argument("Replacement policy must be either 'LRU' or 'LFU'.");
	}
	if (write_back && !write_allocate) {
		throw std::invalid_argument("Write-back caches must be write-allocate.");
	}

	valid = new bool *[_set_count];
	dirty = new bool *[_set_count];
	tag = new uint32_t *[_set_count];
	cnt = new uint32_t *[_set_count];
	for (uint32_t i = 0u; i < _set_count; i++) {
		valid[i] = new bool[_associativity];
		dirty[i] = new bool[_associativity];
		tag[i] = new uint32_t[_associativity];
		cnt[i] = new uint32_t[_associativity];
		for (uint32_t j = 0u; j < _associativity; j++) {
			valid[i][j] = false;
			dirty[i][j] = false;
			tag[i][j] = 0u;
			cnt[i][j] = 0u;
		}
	}
}

Cache::~Cache() {
	for (uint32_t i = 0u; i < _set_count; i++) {
		delete[] valid[i];
		delete[] dirty[i];
		delete[] tag[i];
		delete[] cnt[i];
	}
	delete[] valid;
	delete[] dirty;
	delete[] tag;
	delete[] cnt;
}

void Cache::empty() {
	for (uint32_t i = 0u; i < _set_count; i++) {
		for (uint32_t j = 0u; j < _associativity; j++) {
			valid[i][j] = false;
			dirty[i][j] = false;
			tag[i][j] = 0u;
			cnt[i][j] = 0u;
		}
	}
}

uint32_t Cache::read_1024(uint32_t base_addr, uint32_t stride) {
	if (++_read_count > _read_limit) {
		throw std::runtime_error("Read limit exceeded");
	}
	uint32_t hits = 0u;
	for (uint32_t i = 0u; i < 1024u; i++) {
		hits += _read(base_addr + i * stride);
	}
	return hits;
}

uint32_t Cache::write_1024(uint32_t base_addr, uint32_t stride) {
	if (++_write_count > _write_limit) {
		throw std::runtime_error("Write limit exceeded");
	}
	uint32_t total_latency = 0u;
	for (uint32_t i = 0u; i < 1024u; i++) {
		total_latency += _write(base_addr + i * stride);
	}
	return total_latency;
}

// private methods

uint32_t Cache::_index(uint32_t address) {
	return (address / _block_size) % _set_count;
}
uint32_t Cache::_offset(uint32_t address) {
	return address % _block_size;
}
uint32_t Cache::_tag(uint32_t address) {
	return address / _block_size / _set_count;
}

void Cache::_update_lfu(uint32_t set_index, uint32_t way) {
	cnt[set_index][way]++;
}
uint32_t Cache::_query_lfu(uint32_t set_index) {
	uint32_t lfu_way = 0u;
	uint32_t min_cnt = cnt[set_index][0];
	for (uint32_t i = 1u; i < _associativity; i++) {
		if (cnt[set_index][i] <= min_cnt) { // evict the most recently used one if tie
			min_cnt = cnt[set_index][i];
			lfu_way = i;
		}
	}
	return lfu_way;
}

uint32_t Cache::_query_empty(uint32_t set_index) {
	for (uint32_t i = 0u; i < _associativity; i++) {
		if (!valid[set_index][i]) {
			return i;
		}
	}
	return _associativity; // not found
}
uint32_t Cache::_query_tag(uint32_t set_index, uint32_t tag_value) {
	for (uint32_t i = 0u; i < _associativity; i++) {
		if (valid[set_index][i] && tag[set_index][i] == tag_value) {
			return i;
		}
	}
	return _associativity; // not found
}

uint32_t Cache::_evict(uint32_t set_index) {
	uint32_t way;
	if (_replacement_policy == "LRU") {
		way = _query_lru_student_implementation(set_index);
	} else { // LFU
		way = _query_lfu(set_index);
	}
	if (_write_back && dirty[set_index][way]) {
		// write back to memory (simulated)
		dirty[set_index][way] = false;
	}
	valid[set_index][way] = false;
	tag[set_index][way] = 0u;
	cnt[set_index][way] = 0u;
	return way;
}

uint32_t Cache::_read(uint32_t address) {
	uint32_t set_index = _index(address);
	uint32_t tag_value = _tag(address);

	uint32_t way = _query_tag(set_index, tag_value);
	if (way < _associativity) { // hit
		if (_replacement_policy == "LRU") {
			_update_lru_student_implementation(set_index, way);
		} else { // LFU
			_update_lfu(set_index, way);
		}
		return 1u;
	} else { // miss
		way = _query_empty(set_index);
		if (way == _associativity) { // need to evict
			way = _evict(set_index);
		}
		valid[set_index][way] = true;
		tag[set_index][way] = tag_value;
		if (_replacement_policy == "LRU") {
			_update_lru_student_implementation(set_index, way);
		} else { // LFU
			_update_lfu(set_index, way);
		}
		return 0u;
	}
}

uint32_t Cache::_write(uint32_t address) {
	uint32_t set_index = _index(address);
	uint32_t tag_value = _tag(address);

	if (!_write_allocate) {
		// Write-no-allocate: always miss, do not load into cache
		return writethrough_latency;
	}
	uint32_t way = _query_tag(set_index, tag_value);
	if (way < _associativity) { // hit
		if (_replacement_policy == "LRU") {
			_update_lru_student_implementation(set_index, way);
		} else { // LFU
			_update_lfu(set_index, way);
		}
		if (_write_back) {
			dirty[set_index][way] = true;
			return hit_latency;
		} else {
			return writethrough_latency;
		}
	} else { // miss
		way = _query_empty(set_index);
		if (way == _associativity) { // need to evict
			way = _evict(set_index);
		}
		valid[set_index][way] = true;
		tag[set_index][way] = tag_value;
		if (_replacement_policy == "LRU") {
			_update_lru_student_implementation(set_index, way);
		} else { // LFU
			_update_lfu(set_index, way);
		}
		if (_write_back) {
			dirty[set_index][way] = true;
			return miss_latency;
		} else {
			return miss_latency;
		}
	}
}

Cache *current_cache = nullptr;

} // namespace CACHE
#include "attack.h"
#include "cache.h"
#include <cstdio>

using namespace CACHE;

int main() {
	uint32_t test_block_size = 64_Bytes;
	uint32_t test_associativity = 4u;
	uint32_t test_set_count = 16u;
	std::string test_replacement_policy = "LRU";
	bool test_write_back = true,
		 test_write_allocate = true;
	current_cache = new Cache(test_block_size,
							  test_associativity,
							  test_set_count,
							  test_replacement_policy,
							  test_write_back,
							  test_write_allocate,
							  1000,
							  1000);
	test_replacement_policy = ""; // Hide replacement policy
	attack(1, test_block_size, test_associativity, test_set_count, test_replacement_policy, test_write_back, test_write_allocate);
	delete current_cache;
	return 0;
}
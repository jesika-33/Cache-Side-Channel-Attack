#include "attack.h"
#include <stdexcept>
#include <cstdio>
#include <cmath>

namespace CACHE{
	
void Cache::_update_lru_student_implementation(uint32_t set_index, uint32_t way) {
	// Your implementation here
	// lazy implementation, cnt is used as a recently used counter
	// each time a way is accessed we set cnt to 0, and increment the others
      // the larger cnt is the less recent it was accessed.
	for (uint32_t i = 0; i < _associativity; i++) {
		if (i == way) {
			cnt[set_index][i] = 0;
		} else {
			cnt[set_index][i]++;
		}
	}
}

uint32_t Cache::_query_lru_student_implementation(uint32_t set_index) {
	// Your implementation here
	// the logic becomes almost identical to LFU, the only difference is
	// we evict the one with the max_cnt instead of min_cnt.
	uint32_t lru_way = 0;
	uint32_t max_cnt = cnt[set_index][0];

	for (uint32_t i = 1u; i < _associativity; i++) {
		if (cnt[set_index][i] >= max_cnt) {
			max_cnt = cnt[set_index][i];
			lru_way = i;
		}
	}

	return lru_way;
}

} // namespace CACHE

uint32_t read_1024(uint32_t base_addr, uint32_t stride) {
	if (CACHE::current_cache == nullptr) {
		throw std::runtime_error("Cache not initialized");
	}
	return CACHE::current_cache->read_1024(base_addr, stride);
}

uint32_t write_1024(uint32_t base_addr, uint32_t stride) {
	if (CACHE::current_cache == nullptr) {
		throw std::runtime_error("Cache not initialized");
	}
	return CACHE::current_cache->write_1024(base_addr, stride);
}

void attack(uint32_t test_case,
            uint32_t& block_size,
            uint32_t& associativity,
            uint32_t& set_count,
            std::string& replacement_policy,
            bool& write_back,
            bool& write_allocate) {
      if (block_size == 0u) { 
            /* ######################################################################################
            *                        Stride 1 Probing for Unknown Block Size 
            *
            *           Scope / Domain of searching : [4,32] and {16,32,64,128,256,512}
            * ######################################################################################
            */
            /*  Idea: with stride 1, it will go through 1024 consecutive instruction
            *  Assume block size is i
            *  On the 1st read, addr will be a miss, and it will get i instruction and store it in cache
            *  Hence, for the next  i-1 read, it will be a hit,
            *  But on the ith read, it will again be a miss and so on
            * 
            *  Hence, we saw a pattern that is the 
            *  (Block size-1 / block size) x 1024 is approximately the amount of hit
            *  
            *  The equation can be derived into
            *  block size = 1024 / (1024-hit)
            * 
            *  Note:
            *  -(by observation) the equality is true if block size is a power of 2
            *  -(by observation) program below is only accurate for block size = [4,32] or when block size = power of 2
            *  - A more accurate block size probing (binary search) is used for test case 1-8, 
            *    where read limit is relaxed
            */
            CACHE::current_cache->empty();
            if (test_case >= 1 && test_case <= 8) {
                  uint32_t low = 4;
                  uint32_t high = 512;

                  while (low < high) {
                        uint32_t middle = (high + low) / 2;
                        uint32_t hits = read_1024(0, middle);

                        if (hits == 0) {
                              high = middle;
                        } else {
                              low = middle + 1;
                        }
                  }

                  block_size = low;
            } else if (test_case >= 9 && test_case <= 11) {
                  uint32_t hits = read_1024(0, 1);

                  uint32_t misses = 1024 - hits;
                  if (misses == 0) misses = 1;

                  block_size = std::ceil(1024.0 / misses);
            }
      }
      if (associativity == 0u) {
            /*#######################################################
            *                    ASSOCIATIVITY
            *
            *      Scope / Domain of searching : {1,2,4,8,16}
            *#######################################################
            */

            /* Idea: 
            * - On the first read_1024, load all unique block instruction to cache to the same set (0)
                  - By assignement requirement: the maximum amount of address that can be on cache at the same time is 2^21, 
                  so stride cannot be less than 2^21, otherwise might risk reading an instruction that has been loaded
                  - Dont want too much unique set to reduce complexity 
                  - Hence We use 2 different strides for each read function
                        1. 2^30 (4 unique block of address) 
                        --> if associativity = 1 or 2--> unique block of address gets replaced every time--> always miss (hit =0)
                        --> if associativity = 4/ 8/ 16 --> all unique block of address gets stored in cache and does not get replaced
                                                      --> all hit other than initial
                        2. 2^28 (16 unique block of address) (only for associativity larger than or equal to 4)
                        --> if associativity = 4 --> hit = 1 (hit on base address)
                        --> if associativity = 8 --> hit = 2 (hit on base address  and 2^30 is still available from previous iteration)
                        --> if associativity = 16 --> miss on 8 first initial load, and 4 hit from previous read, the rest of thre iteration arrer all hit
                        3. 2^31 (2 unique block of address) (only for associativity amaller than 4)
                        --> if associativity = 1 --> hit = 0 (since all block address are unique, the same cache gets replaced every time)
                        --> if associativity = 2 --> miss = 2 (to load on the unique block of address, 
                                                      NOTE : the address from before gets replaced because the last address read are 2^31(LRU) and 3* 2^30)
            */

            CACHE::current_cache->empty();

            uint32_t stride1 = 1u << 30; 
            uint32_t hits_1 = read_1024(0, stride1);

            associativity = 1;

            if (hits_1 ==0){
                  uint32_t stride2 = 1u << 31; 
                  uint32_t hits_2 = read_1024(0, stride2);
                  if(hits_2 == 1022) associativity =2;
            } else if(hits_1 == 1020){
                  uint32_t stride3 = 1u << 28; 
                  uint32_t hits_3 = read_1024(0, stride3);
                  if (hits_3 == 1)associativity = 4;
                  else if(hits_3 == 2) associativity = 8;
                  else if(hits_3 == 1012) associativity = 16;
            }
      }
      if (set_count == 0u) { // set count unknown
            /*############################################################
            *                     SET COUNT POLICY
            *
            *    Scope / Domain of searching : 1,2,4,8,16,32,64,128,256}
            *############################################################
            */
           /* Idea:
             - Want to iterate over each set,but every time we go to the same set, 
               we have the same blocks of data 
            * 
            */
      }
      if (replacement_policy.empty()) { // replacement policy unknown
            /*############################################################
            *                    REPLACEMENT POLICY
            *
            *          Scope / Domain of searching : {LRU, LFU}
            *############################################################
            */
            /* Idea: 
            * - For all read, read data blocks that belong to the same set
            * - 1st read = load data and gain frequency count
                        --> stride = 0
                        --> 0 is chosen because the minimum number of associativyt is 0, 
                        hence for any associativity the specific  loaded block of data stay in cache
                        --> Note : frequency count = 1024
                  - 2nd read = load unique blocks of data 
                        --> larger stride --> have more than 16 unique blocks of data 
                                          so that all associativity gets replaced
                        --> stride requirement : less than 2^28 (to get all blocks replaced), 
                                                more than or equal to 2^27(go to base address at least twice)
                        --> In this case, we choose 2^27 for simplicity
                  - If Replacement Policy: "LRU"
                        --> 2nd read's hit = 1 --> that is during the first iteration of 2nd read (base address is in memory)
                  - If Replacement Policy: "LFU"
                        --> 2nd read's hit = at least 2 (assuming associativity >1) 
            *
            */
            CACHE::current_cache->empty();

            replacement_policy = "LRU";
            uint32_t load_read = read_1024(0, 0);
            uint32_t check_data = read_1024(0,1<<27);

            if (check_data == 1){
                replacement_policy = "LRU";
            } else{
                replacement_policy = "LFU";
            }
      }
      if (write_back && !write_allocate) { // write policy unknown
            /*############################################################
            *                        WRITE POLICY
            *
            *  Scope / Domain of searching : 
            * (1) write-back & write-allocation, (2) write-through & write
                  allocation, (3) write-through & write-no-allocation.
            *############################################################
            */
            /* Idea: stride : 3 ensures that there will be a hit no matter the block size, considering block size is [4, 512]
                  --> if latency==  20 *1024 --> (3) write through, no allocation (has latency of 20 for both miss and hit, 
                                                and that cache is accessed 1024 times)
                  --> if latency < 20 *1024 -->  (1) write-back & write-allocation (since cache hit of this policy 
                                                has less latency, compared to policy (3) whom has same latency for cache miss)
                  --> else                  --> (2)write-through & write allocation (bigger latency compared to policy (3), 
                                                no matter cache hit or cache miss)
            */
            CACHE::current_cache->empty();

            write_allocate = false;
            write_back = false;
            uint32_t latency = write_1024(0, 3);

            uint32_t wt_wnoal_lat = 20480; /// 1024* 20
            if (latency == wt_wnoal_lat) {
              write_allocate = false;
              write_back = false;
            } else if (latency< wt_wnoal_lat) {
              write_allocate = true;
              write_back = true;
            }else {
              write_back = false;
              write_allocate = true;
            }
      }

      // FINAL CHECK
      // printf("BLOCK SIZE: %d, ASSOCIATIVITY: %d, SET COUNT: %d, POLICY: %s, WRITEBACK: %d, WALLOC: %d", block_size, associativity, set_count, replacement_policy, write_back, write_allocate);
      return;
}
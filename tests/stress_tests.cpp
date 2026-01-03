#include "kv_store.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <chrono>

static std::string random_key(std::mt19937& rng) {
	static const char chars[] =
		"abcdefghijklmnopqrstuvwxyz0123456789";
	std::uniform_int_distribution<> len_dist(5, 15);
	std::uniform_int_distribution<> char_dist(0, sizeof(chars) - 2);

	int len = len_dist(rng);
	std::string key;
	key.reserve(len);

	for (int i = 0; i < len; ++i) {
		key.push_back(chars[char_dist(rng)]);
	}
	return key;
}

void stress_test(EvictionPolicyType policy) {
	constexpr std::size_t CAPACITY = 1000;
	constexpr int OPERATIONS = 150000;

	KVStore store(CAPACITY, policy);

	std::mt19937 rng(42);
	std::uniform_int_distribution<> op_dist(0, 2);
	std::uniform_int_distribution<> ttl_dist(-1, 3);

	std::vector<std::string> keys;
	keys.reserve(5000);

	for (int i = 0; i < OPERATIONS; ++i) {
		int op = op_dist(rng);

		if (op == 0) { // SET
			std::string key = random_key(rng);
			std::string value = "value_" + std::to_string(i);
			int ttl = ttl_dist(rng);

			store.set(key, value, ttl);
			keys.push_back(key);
		}
		else if (op == 1 && !keys.empty()) { // GET
			const std::string& key = keys[rng() % keys.size()];
			std::string value;
			store.get(key, value); // value validity checked implicitly
		}
		else if (op == 2 && !keys.empty()) { // DEL
			const std::string& key = keys[rng() % keys.size()];
			store.del(key);
		}

		// Invariant: size must never exceed capacity
		assert(store.size() <= CAPACITY);
	}

	std::cout << "[STRESS TEST PASSED] "
		<< (policy == EvictionPolicyType::LRU ? "LRU" : "LFU")
		<< std::endl;
}

int main() {
	stress_test(EvictionPolicyType::LRU);
	stress_test(EvictionPolicyType::LFU);
	return 0;
}

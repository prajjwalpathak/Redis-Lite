#include "kv_store.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

#define TEST(name) \
    std::cout << "[TEST] " << name << " ... "; \
    std::cout.flush();

#define PASS() \
    std::cout << "PASSED\n";

void test_basic_set_get() {
	TEST("Basic SET / GET");

	KVStore store(10, EvictionPolicyType::LRU);

	assert(store.set("a", "1"));
	std::string value;
	assert(store.get("a", value));
	assert(value == "1");

	PASS();
}

void test_delete() {
	TEST("DEL operation");

	KVStore store(10, EvictionPolicyType::LRU);

	store.set("a", "1");
	assert(store.del("a"));

	std::string value;
	assert(!store.get("a", value));

	PASS();
}

void test_ttl_expiration() {
	TEST("TTL expiration");

	KVStore store(10, EvictionPolicyType::LRU);

	store.set("temp", "123", 1); // 1 second TTL
	std::this_thread::sleep_for(1500ms);

	std::string value;
	assert(!store.get("temp", value));
	assert(store.size() == 0);

	PASS();
}

void test_lru_eviction() {
	TEST("LRU eviction");

	KVStore store(2, EvictionPolicyType::LRU);

	store.set("a", "1");
	store.set("b", "2");

	std::string value;
	store.get("a", value); // a becomes MRU

	store.set("c", "3"); // should evict b

	assert(store.get("a", value));
	assert(value == "1");

	assert(!store.get("b", value));
	assert(store.get("c", value));

	PASS();
}

void test_lfu_eviction() {
	TEST("LFU eviction");

	KVStore store(2, EvictionPolicyType::LFU);

	store.set("a", "1");
	store.set("b", "2");

	std::string value;
	store.get("a", value);
	store.get("a", value); // freq(a) = 3, freq(b) = 1

	store.set("c", "3"); // should evict b

	assert(store.get("a", value));
	assert(value == "1");

	assert(!store.get("b", value));
	assert(store.get("c", value));

	PASS();
}

void test_thread_safety() {
	TEST("Thread safety");

	KVStore store(100, EvictionPolicyType::LRU);

	auto worker = [&store](int id) {
		for (int i = 0; i < 100; ++i) {
			store.set("k" + std::to_string(id * 100 + i),
				std::to_string(i));
		}
		};

	std::vector<std::thread> threads;
	for (int i = 0; i < 4; ++i) {
		threads.emplace_back(worker, i);
	}

	for (auto& t : threads) {
		t.join();
	}

	assert(store.size() <= 100);

	PASS();
}

int main() {
	std::cout << "Running Redis-Lite tests...\n\n";

	test_basic_set_get();
	test_delete();
	test_ttl_expiration();
	test_lru_eviction();
	test_lfu_eviction();
	test_thread_safety();

	std::cout << "\nAll tests PASSED\n";
	return 0;
}

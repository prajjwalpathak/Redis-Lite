#include "kv_store.h"
#include <iostream>
#include <thread>

int main() {
	// Create a Redis-Lite store with capacity = 2 and LRU eviction
	// LRU here could be replace with LFU
	KVStore store(2, EvictionPolicyType::LRU);

	// SET keys
	store.set("user:1", "Alice");
	store.set("user:2", "Bob");

	// GET keys
	std::string value;
	if (store.get("user:1", value)) {
		std::cout << "user:1 = " << value << "\n";
	}

	// This access makes "user:1" most recently used
	// Next insert will evict "user:2"
	store.set("user:3", "Charlie");

	if (!store.get("user:2", value)) {
		std::cout << "user:2 was evicted (LRU)\n";
	}

	std::cout << "Current size: " << store.size() << "\n";

	if (store.get("user:3", value)) {
		std::cout << "user:3 = " << value << "\n";
	}

	if (store.get("user:1", value)) {
		std::cout << "user:1 = " << value << "\n";
	}

	// TTL example (key expires after 2 seconds)
	store.set("temp", "123", 2);

	if (store.get("temp", value)) {
		std::cout << "temp = " << value << "\n";
	}

	std::cout << "Current size: " << store.size() << "\n";

	std::this_thread::sleep_for(std::chrono::seconds(3));

	if (!store.get("temp", value)) {
		std::cout << "temp expired due to TTL\n";
	}

	std::cout << "Current size: " << store.size() << "\n";

	return 0;
}

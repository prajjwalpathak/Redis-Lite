#pragma once

#include<string>
#include<cstddef>

// Eviction policy selection at initialization
enum class EvictionPolicyType {
	LRU,
	LFU
};

/*
 * KVStore
 *
 * A thread-safe in-memory key-value store supporting:
 * - SET / GET / DEL
 * - TTL (key expiration)
 * - Configurable eviction policy (LRU or LFU)
 *
 * NOTE:
 * - This is a library-style interface.
 * - All methods are thread-safe.
 */
class KVStore {
public:
	// Construct a KV store with a fixed capacity and eviction policy
	KVStore(std::size_t capacity, EvictionPolicyType policy);

	// Disable copy (simplifies ownership & correctness)
	KVStore(const KVStore&) = delete;
	KVStore& operator=(const KVStore&) = delete;

	// Allow move if needed later
	KVStore(KVStore&&) = default;
	KVStore& operator=(KVStore&&) = default;

	// Insert or update a key with optional TTL (in seconds)
	// ttl_seconds < 0 means no expiration
	bool set(const std::string& key,
		const std::string& value,
		int ttl_seconds = -1);

	// Retrieve value for a key
	// Returns true if key exists and is not expired
	bool get(const std::string& key, std::string& out_value);

	// Delete a key if it exists
	bool del(const std::string& key);

	// Current number of valid (non-expired) keys
	std::size_t size() const;

	// Destructor
	~KVStore();

private:
	// Forward declaration hides implementation details
	struct Impl;
	Impl* impl_;
};

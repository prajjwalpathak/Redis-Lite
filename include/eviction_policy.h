#pragma once

#include <string>

/*
 * EvictionPolicy
 *
 * Abstract base class for cache eviction strategies.
 * Implementations must guarantee O(1) average-time eviction.
 *
 * KVStore calls these hooks to notify the policy about key events.
 */
class EvictionPolicy {
public:
	virtual ~EvictionPolicy() = default;

	// Called when a key is successfully accessed
	virtual void on_get(const std::string& key) = 0;

	// Called when a key is inserted or updated
	virtual void on_set(const std::string& key) = 0;

	// Called when a key is explicitly removed or expired
	virtual void on_del(const std::string& key) = 0;

	// Returns the key to evict when capacity is exceeded
	// Assumes at least one key exists
	virtual std::string evict_key() = 0;
};

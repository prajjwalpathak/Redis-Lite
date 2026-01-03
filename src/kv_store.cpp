#include "kv_store.h"

#include <unordered_map>
#include <string>
#include <mutex>
#include <chrono>
#include <list>


using Clock = std::chrono::steady_clock;

//
// Internal Entry Structure
//
struct Entry {
	std::string value;
	Clock::time_point expiry; // time_point::max() => no TTL
};

//
// pImpl Definition
//
struct KVStore::Impl {
	std::size_t capacity;
	EvictionPolicyType policy;

	std::unordered_map<std::string, Entry> store;

	// LRU
	std::list<std::string> lru_list;
	std::unordered_map<std::string, std::list<std::string>::iterator> lru_map;

	// LFU
	std::unordered_map<std::string, std::size_t> freq;

	mutable std::mutex mutex;

	bool isExpired(const Entry& e) const {
		if (e.expiry == Clock::time_point::max())
			return false;
		return Clock::now() >= e.expiry;
	}

	void touchLRU(const std::string& key) {
		auto it = lru_map.find(key);
		if (it != lru_map.end()) {
			lru_list.erase(it->second);
		}
		lru_list.push_front(key);
		lru_map[key] = lru_list.begin();
	}

	void removeKey(const std::string& key) {
		store.erase(key);

		// LRU cleanup
		auto lit = lru_map.find(key);
		if (lit != lru_map.end()) {
			lru_list.erase(lit->second);
			lru_map.erase(lit);
		}

		// LFU cleanup
		freq.erase(key);
	}

	void evictIfNeeded() {
		if (store.size() <= capacity)
			return;

		if (policy == EvictionPolicyType::LRU) {
			const std::string victim = lru_list.back();
			lru_list.pop_back();
			lru_map.erase(victim);
			store.erase(victim);
			return;
		}

		// LFU eviction (simple)
		std::string victim;
		std::size_t min_freq = SIZE_MAX;

		for (const auto& [key, f] : freq) {
			if (f < min_freq) {
				min_freq = f;
				victim = key;
			}
		}

		removeKey(victim);
	}
};


//
// KVStore Implementation
//

KVStore::KVStore(std::size_t capacity, EvictionPolicyType policy)
	: impl_(new Impl{ capacity, policy }) {
}

KVStore::~KVStore() {
	delete impl_;
}

bool KVStore::set(const std::string& key,
	const std::string& value,
	int ttl_seconds) {
	std::lock_guard<std::mutex> lock(impl_->mutex);

	Entry entry;
	entry.value = value;
	entry.expiry = (ttl_seconds < 0)
		? Clock::time_point::max()
		: Clock::now() + std::chrono::seconds(ttl_seconds);

	bool exists = impl_->store.count(key);

	impl_->store[key] = std::move(entry);

	if (impl_->policy == EvictionPolicyType::LRU) {
		impl_->touchLRU(key);
	}
	else {
		impl_->freq[key]++;   // LFU increment
	}

	if (!exists) {
		impl_->evictIfNeeded();
	}

	return true;
}


bool KVStore::get(const std::string& key, std::string& out_value) {
	std::lock_guard<std::mutex> lock(impl_->mutex);

	auto it = impl_->store.find(key);
	if (it == impl_->store.end())
		return false;

	if (impl_->isExpired(it->second)) {
		impl_->removeKey(key);
		return false;
	}

	if (impl_->policy == EvictionPolicyType::LRU) {
		impl_->touchLRU(key);
	}
	else {
		impl_->freq[key]++;   // LFU increment
	}

	out_value = it->second.value;
	return true;
}


bool KVStore::del(const std::string& key) {
	std::lock_guard<std::mutex> lock(impl_->mutex);

	if (impl_->store.count(key) == 0)
		return false;

	impl_->removeKey(key);
	return true;
}


std::size_t KVStore::size() const {
	std::lock_guard<std::mutex> lock(impl_->mutex);

	std::size_t count = 0;
	auto now = Clock::now();

	for (auto it = impl_->store.begin(); it != impl_->store.end(); ) {
		if (it->second.expiry != Clock::time_point::max() &&
			it->second.expiry <= now) {
			const std::string key = it->first;
			++it;
			impl_->removeKey(key);
		}
		else {
			++count;
			++it;
		}
	}
	return count;
}
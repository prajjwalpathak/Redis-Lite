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

	// LRU structures
	std::list<std::string> lru_list;
	std::unordered_map<std::string, std::list<std::string>::iterator> lru_map;

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

	void evictIfNeeded() {
		if (store.size() <= capacity)
			return;

		const std::string& victim = lru_list.back();
		store.erase(victim);
		lru_map.erase(victim);
		lru_list.pop_back();
	}

	void removeKey(const std::string& key) {
		store.erase(key);
		auto it = lru_map.find(key);
		if (it != lru_map.end()) {
			lru_list.erase(it->second);
			lru_map.erase(it);
		}
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
	impl_->touchLRU(key);

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

	impl_->touchLRU(key);
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
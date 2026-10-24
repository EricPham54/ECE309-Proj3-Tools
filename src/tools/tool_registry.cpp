// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

#include <cstdint>
#include <string>
#include <utility>

ToolRegistry::ToolRegistry() : buckets_(kInitialBuckets) {} // create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;                                                           // FNV-1a (64-bit) hashing method from Appendix A of P3_spec.md
    for (unsigned char c : key) {                                                                       // Hashes each character from the given key
        h ^= c;
        h *= 1099511628211ULL;                 
    }
    return static_cast<std::size_t>(h);                                                                 // returns the size_t type converted h (changes depending on how big given key is and what it contains)
}

std::size_t ToolRegistry::index_for(std::string_view key, std::size_t bucket_count) noexcept {
    return hash(key) % bucket_count;                                                                    // returns the bucket index by modding the hash of the key with the number of buckets
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) return false;                                                                            // returns false if tool is nullptr

    const std::string key = tool->name();                                                               // access the name of the tool to see if it already exists
    if (find(key) != nullptr) return false;                                                             // returns false if the tools already exists in the hash table

    while (static_cast<double>(size_ + 1) / static_cast<double>(buckets_.size()) > kMaxLoad)            // if the next add would cause the current load factor to be more than the max load factor, then rehash/resize the hash
        rehash();

    buckets_[index_for(key, buckets_.size())].push_back(std::move(tool));                               // pushes the tool to the back of the bucket index vector
    ++size_;                                                                                            // increases the size member data of the hash table
    return true;                                                                                        // returns true if the tool was able to add the tool to the hash table
}

Tool* ToolRegistry::find(std::string_view name) const {
    const auto& bucket = buckets_[index_for(name, buckets_.size())];                                    // gets the theoretical bucket index for the name of the tool
    for (const auto& t : bucket) {                                                                      // goes through the chained bucket index vector to see if the element with that name exists
        if (t->name() == name) return t.get();
    }
    return nullptr;                                                                                     // returns nullptr if the element with the name doesn't exist
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;                                                                                       // returns the number of elements (size_) in the hash table
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {                                                               // calls fn once per registered tool in the hash table through the use of a nested for-loop
        for (const auto& t : bucket) fn(*t);
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();                                                                             // returns the number of buckets (indexes of chained/unchained) in the hash table
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());                           // returns the load factor by dividing the number of occupied buckets by the number of buckets that exist
}


std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;                                                                            // sets variable to see the length of the longest chain
    for (const auto& bucket : buckets_) {                                                               // goes through each bucket (addresses of vectors for chaining) to see which one is the longest using an if statement
        if (bucket.size() > longest) longest = bucket.size();
    }
    return longest;                                                                                     // returns the longest chain variable
}

void ToolRegistry::rehash() {
    const std::size_t new_count = buckets_.size() * 2;                                                  // creates a size_t that is twice the number of the current amount of buckets in the hash table
    std::vector<std::vector<std::unique_ptr<Tool>>> newTable(new_count);                                // creates new vector of vectors for the hash table

    for (auto& bucket : buckets_) {                                                                     // uses a nexted for-loop to move the unique pointers of the tool elements from the original hash table into the new hash table
        for (auto& t : bucket) {
            const std::size_t i = index_for(t->name(), new_count);
            newTable[i].push_back(std::move(t));
        }
    }
    buckets_ = std::move(newTable);                                                                     // moves the newTable vector into the buckets_ vector variable
}

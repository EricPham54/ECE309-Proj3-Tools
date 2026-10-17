// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.

#include "tools/tool_registry.h"

#include <cstdint>
#include <string>
#include <utility>

ToolRegistry::ToolRegistry() : buckets_(kInitialBuckets) {} // create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL; 
    for (unsigned char c : key) {
        h ^= c;
        h *= 1099511628211ULL;                 
    }
    return static_cast<std::size_t>(h);
}

std::size_t ToolRegistry::index_for(std::string_view key, std::size_t bucket_count) noexcept {
    return hash(key) % bucket_count;
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) return false;

    const std::string key = tool->name();
    if (find(key) != nullptr) return false;     

    while (static_cast<double>(size_ + 1) / static_cast<double>(buckets_.size()) > kMaxLoad)
        rehash();

    buckets_[index_for(key, buckets_.size())].push_back(std::move(tool));
    ++size_;
    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    const auto& bucket = buckets_[index_for(name, buckets_.size())];
    for (const auto& t : bucket) {
        if (t->name() == name) return t.get();
    }
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {
        for (const auto& t : bucket) fn(*t);
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());
}


std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;
    for (const auto& bucket : buckets_) {
        if (bucket.size() > longest) longest = bucket.size();
    }
    return longest;
}

void ToolRegistry::rehash() {
    const std::size_t new_count = buckets_.size() * 2;
    std::vector<std::vector<std::unique_ptr<Tool>>> fresh(new_count);

    for (auto& bucket : buckets_) {
        for (auto& t : bucket) {
            const std::size_t i = index_for(t->name(), new_count);
            fresh[i].push_back(std::move(t));
        }
    }
    buckets_ = std::move(fresh);
}

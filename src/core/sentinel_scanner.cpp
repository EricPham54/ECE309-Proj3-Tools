#include "core/sentinel_scanner.h"
#include <algorithm>


// SentinelScanner constructor
SentinelScanner::SentinelScanner(std::string sentinel)
    : sentinel_(std::move(sentinel)) {}


// SentinelScanner feed function to detect if the chunk + current pending_ variable will equal to the
// sentinel_. If it chunk + current pending_ doesn't contain the full sentinel, then any part of the
// sentinel_, if any, will be put back into the pending_ variable and be used to check with chunk later
// for the sentinel_
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
// Appends chunk to pending_
    std::string combined = pending_ + std::string(chunk);
    pending_.clear();

// Checks if the combined string contains the sentinel
    std::size_t pos = combined.find(sentinel_);
    if (pos != std::string::npos) {
        std::string safe = combined.substr(0, pos);
        return Out{std::move(safe), true};
    }

// If combined doesn't contain the sentinel, then this will check if the combined will contain the start
// of the sneintel_. If combined does conotain the start of the sentinel, then pending_ will hold it. 
    std::size_t bound;
    if (sentinel_.empty()) {
        bound = 0;
    } 
    else {
        bound = sentinel_.size() - 1;
    }
    std::size_t max_hold = std::min(bound, combined.size());
    for (std::size_t len = max_hold; len > 0; len--) {
        std::string_view tail = std::string_view(combined).substr(combined.size() - len);
        std::string_view prefix = std::string_view(sentinel_).substr(0, len);
        if (tail == prefix) {
            pending_ = combined.substr(combined.size() - len);
            std::string safe = combined.substr(0, combined.size() - len);
            return Out{std::move(safe), false};
        }
    }


// If no part of the sentinel_ is found in the combined string, then return with false for found sentinel_
    return Out{std::move(combined), false};
}


// Clear out pending_ and return with false for found sentinel_
SentinelScanner::Out SentinelScanner::flush() {
    std::string leftover = std::move(pending_);
    pending_.clear();
    return Out{std::move(leftover), false};
}
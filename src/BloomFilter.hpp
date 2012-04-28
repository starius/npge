/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_BLOOM_FILTER_HPP_
#define BR_BLOOM_FILTER_HPP_

#include <cstddef>
#include <vector>
#include <string>

namespace br {

class BloomFilter {
public:
    typedef std::size_t size_t;

    BloomFilter(size_t bits, size_t hashes);

    size_t bits() const;

    size_t hashes() const;

    void add(const char* start, size_t length);

    void add(const std::string& member);

    bool test(const char* start, size_t length) const;

    bool test(const std::string& member) const;

private:
    std::vector<bool> bits_;
    std::vector<size_t> hash_mul_;

    size_t make_index(size_t hash, const char* start, size_t length) const;
};

}

#endif


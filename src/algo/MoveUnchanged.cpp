/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/cast.hpp>
#include <boost/foreach.hpp>

#include "MoveUnchanged.hpp"
#include "BlockSet.hpp"
#include "global.hpp"
#include "block_hash.hpp"

namespace bloomrepeats {

MoveUnchanged::MoveUnchanged() {
    declare_bs("other", "Where unchanged blocks are looked for");
    declare_bs("target", "Where unchanged blocks are moved to");
    set_block_set_name("other");
}

void MoveUnchanged::clear() {
    hashes_.clear();
}

struct MoveUnchangedData : public ThreadData {
    std::vector<uint32_t> hashes_;
    Blocks moved_;
};

ThreadData* MoveUnchanged::before_thread_impl() const {
    return new MoveUnchangedData;
}

void MoveUnchanged::process_block_impl(Block* b,
                                       ThreadData* d) const {
    MoveUnchangedData* data;
    data = boost::polymorphic_downcast<MoveUnchangedData*>(d);
    uint32_t hash = block_hash(b);
    if (hashes_.find(hash) != hashes_.end()) {
        // move
        data->moved_.push_back(b);
    } else {
        // memorize hash
        data->hashes_.push_back(hash);
    }
}

void MoveUnchanged::after_thread_impl(ThreadData* d) const {
    MoveUnchangedData* data;
    data = boost::polymorphic_downcast<MoveUnchangedData*>(d);
    BlockSet& t = *block_set();
    BlockSet& o = *other();
    BOOST_FOREACH (Block* b, data->moved_) {
        o.detach(b);
        t.insert(b);
    }
    BOOST_FOREACH (uint32_t hash, data->hashes_) {
        hashes_.insert(hash);
    }
}

const char* MoveUnchanged::name_impl() const {
    return "Move unchanged blocks from other to target";
}

}

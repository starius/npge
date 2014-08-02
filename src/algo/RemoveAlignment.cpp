/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/foreach.hpp>

#include "RemoveAlignment.hpp"
#include "BlockSet.hpp"
#include "Block.hpp"
#include "Fragment.hpp"

namespace npge {

RemoveAlignment::RemoveAlignment() {
    declare_bs("target", "Target blockset");
}

void RemoveAlignment::run_impl() const {
    BOOST_FOREACH (Block* b, *block_set()) {
        b->remove_alignment();
    }
}

const char* RemoveAlignment::name_impl() const {
    return "Remove alignment from blocks";
}

}


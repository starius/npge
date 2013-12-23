/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2013 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/foreach.hpp>

#include "RemoveNames.hpp"
#include "BlockSet.hpp"
#include "Block.hpp"
#include "Sequence.hpp"

namespace bloomrepeats {

RemoveNames::RemoveNames() {
    add_opt("remove-blocks-names", "Remove blocks names", true);
    add_opt("remove-seqs-names", "Remove seqences names", true);
}

bool RemoveNames::run_impl() const {
    if (opt_value("remove-blocks-names").as<bool>()) {
        BOOST_FOREACH (Block* block, *block_set()) {
            block->set_name("");
        }
    }
    if (opt_value("remove-seqs-names").as<bool>()) {
        BOOST_FOREACH (SequencePtr seq, block_set()->seqs()) {
            seq->set_name("");
        }
    }
    return true;
}

const char* RemoveNames::name_impl() const {
    return "Remove all blocks and/or sequences names";
}

}

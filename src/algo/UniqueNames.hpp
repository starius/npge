/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_UNIQUE_NAMES_HPP_
#define BR_UNIQUE_NAMES_HPP_

#include "BlocksJobs.hpp"

namespace bloomrepeats {

/** Set unique names to all blocks of this block set.
If name is not default and not unique:
 - "_num" is appended with num minimum number to make name unique.

If (name is default or "") and not unique:
 - block_name() is used, if name is null.
 - Block::set_random_name() is called untill the name is unique.

If name of sequece is empty or not unique, it is changed to random.
*/
class UniqueNames : public BlocksJobs {
public:
    /** Constructor */
    UniqueNames();

protected:
    void initialize_work() const;
    void process_block_impl(Block* block, ThreadData* data) const;
    void finish_work_impl() const;
    const char* name_impl() const;

private:
    mutable int genomes_;
};

}

#endif


/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_FRAGMENTS_EXTENDER_HPP_
#define BR_FRAGMENTS_EXTENDER_HPP_

#include "BlocksJobs.hpp"

namespace bloomrepeats {

class ExternalAligner;

/** Move block's boundaries and align only new parts.
Blocks without alignment and blocks of <= 2 fragments are not changed.
*/
class FragmentsExtender : public BlocksJobs {
public:
    /** Constructor */
    FragmentsExtender();

    /** Extend one block */
    void extend(Block* block, const std::string& tmp_in,
                const std::string& tmp_out) const;

protected:
    void initialize_work_impl() const;
    ThreadData* before_thread_impl() const;
    void process_block_impl(Block* block, ThreadData*) const;
    void after_thread_impl(ThreadData* data) const;
    const char* name_impl() const;

private:
    ExternalAligner* aligner_;
};

}

#endif

/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_CONSENSUS_TREE_HPP_
#define BR_CONSENSUS_TREE_HPP_

#include "global.hpp"
#include "Processor.hpp"

namespace bloomrepeats {

class BranchGenerator;

/** Print consensus tree.

PrintTree is used to build tree.
*/
class ConsensusTree : public Processor {
public:
    /** Constructor */
    ConsensusTree();

protected:
    bool run_impl() const;

private:
    BranchGenerator* branch_generator_;
};

}

#endif

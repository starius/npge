/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_PRINT_MUTATIONS_HPP_
#define BR_PRINT_MUTATIONS_HPP_

#include <boost/function.hpp>

#include "AbstractOutput.hpp"
#include "global.hpp"

namespace bloomrepeats {

/** Mutation description */
struct Mutation {
    Fragment* fragment;
    int start;
    int stop;
    char change;
};

/** Function processing a mutation */
typedef boost::function<void(const Mutation&)> MutationHandler;

/** Finds all mutations in block */
class PrintMutations : public AbstractOutput {
public:
    /** Constructor */
    PrintMutations();

    /** Finds mutations and calls f() for each mutation */
    void find_mutations(const Block* block, const MutationHandler& f) const;

    /** Print table block - fr - start_pos - stop_pos - change */
    void print_block(std::ostream& o, Block* block) const;

    void print_header(std::ostream& o) const;
};

}

#endif

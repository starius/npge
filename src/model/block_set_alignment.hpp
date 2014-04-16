/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_BLOCK_SET_ALIGNMENT_HPP_
#define BR_BLOCK_SET_ALIGNMENT_HPP_

#include <iosfwd>
#include <map>
#include <vector>

#include "global.hpp"

namespace bloomrepeats {

/** One row of block set alignment */
struct BSRow {
    /** Constructor */
    BSRow();

    /** Ori of row in relation to sequence */
    int ori;

    /** Array of fragments, 0 for gap */
    Fragments fragments;
};

/** Block set alignment */
typedef std::map<Sequence*, BSRow> BSA;

/** Vector of block set alignment */
typedef std::vector<BSA> BSAs;

/** Return length of block set alignment */
int bsa_length(const BSA& bsa);

/** Return is all sequences from bsa are circular */
bool bsa_is_circular(const BSA& bsa);

}

#endif


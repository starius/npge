/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_REMOVE_NAMES_HPP_
#define NPGE_REMOVE_NAMES_HPP_

#include "Processor.hpp"

namespace npge {

/** Set names of all sequences/blocks to empty */
class RemoveNames : public Processor {
public:
    /** Constructor */
    RemoveNames();

protected:
    void run_impl() const;

    const char* name_impl() const;
};

}

#endif


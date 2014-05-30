/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_PRINT_BLOCK_SET_ALIGNMENT_HPP_
#define BR_PRINT_BLOCK_SET_ALIGNMENT_HPP_

#include "global.hpp"
#include "Processor.hpp"
#include "FileWriter.hpp"

namespace npge {

/** Print block set alignment */
class PrintBSA : public Processor {
public:
    /** Constructor */
    PrintBSA();

protected:
    void run_impl() const;

    const char* name_impl() const;

private:
    FileWriter file_writer_;
};

}

#endif


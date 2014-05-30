/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2013 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_FILE_REMOVER_HPP_
#define BR_FILE_REMOVER_HPP_

#include "Processor.hpp"

namespace npge {

/** Removes file */
class FileRemover : public Processor {
public:
    /** Constructor */
    FileRemover();

protected:
    void run_impl() const;
    const char* name_impl() const;
};

}

#endif


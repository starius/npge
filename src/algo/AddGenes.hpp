/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_ADD_GENES_HPP_
#define BR_ADD_GENES_HPP_

#include "Processor.hpp"
#include "FileReader.hpp"

namespace bloomrepeats {

/** Add genes from EBI genes description.
Sequence accession numbers are taken from Sequence.ac().
*/
class AddGenes : public Processor, public FileReader {
protected:
    void add_options_impl(po::options_description& desc) const;

    void apply_options_impl(const po::variables_map& vm);

    bool run_impl() const;

    const char* name_impl() const;
};

}

#endif

/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_ADD_BLOCKS_HPP_
#define BR_ADD_BLOCKS_HPP_

#include "Processor.hpp"
#include "FileReader.hpp"
#include "SeqStorage.hpp"
#include "RowStorage.hpp"

namespace bloomrepeats {

/** Add blocks and sequences to the block set.
If there are no files with sequences specified in --in-seqs,
then sequences are recovered from files with blocks.
In this case blocks must cover sequences entirely.

Input sequences can be passed to, before fragments.

See stream >> block_set, stream >> alignment_row.
*/
class AddBlocks : public Processor, public FileReader,
    public RowStorage, public SeqStorage {
public:
    /** Constructor.
    \param keep_alignment If alignments is extracted too.
    \param row_type Type of alignment rows.
    \param seq_type Type of sequences.
        If not NO_SEQUENCE, then sequences will be read as well.
    */
    AddBlocks(bool keep_alignment = false,
              RowType row_type = COMPACT_ROW,
              SequenceType seq_type = COMPACT_SEQUENCE);

protected:
    /** Add options to options description */
    void add_options_impl(po::options_description& desc) const;

    /** Apply options from variables map */
    void apply_options_impl(const po::variables_map& vm);

    /** Apply the action */
    bool run_impl() const;

    const char* name_impl() const;
};

}

#endif


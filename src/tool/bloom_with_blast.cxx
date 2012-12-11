/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "process.hpp"
#include "Pipe.hpp"
#include "AddSequences.hpp"
#include "AnchorFinder.hpp"
#include "CleanUp.hpp"
#include "AddBlastBlocks.hpp"
#include "CheckNoOverlaps.hpp"
#include "UniqueNames.hpp"
#include "Output.hpp"

using namespace bloomrepeats;

class BloomWithBlastPipe : public Pipe {
public:
    BloomWithBlastPipe() {
        set_empty_block_set();
        add(new AddSequences);
        add(new AnchorFinder);
        add(new CleanUp);
        add(new AddBlastBlocks);
        add(new CleanUp);
        add(new CheckNoOverlaps);
        add(new UniqueNames);
        add(new Output);
    }
};

int main(int argc, char** argv) {
    return process(argc, argv, new BloomWithBlastPipe,
                   "Find and expand anchors, import blast hits", "in-seqs");
}


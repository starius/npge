/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "process.hpp"
#include "Pipe.hpp"
#include "AddSequences.hpp"
#include "AddBlocks.hpp"
#include "ResolveBlast.hpp"
#include "CheckNoOverlaps.hpp"
#include "UniqueNames.hpp"
#include "Output.hpp"

using namespace bloomrepeats;

class ResolveBlastPipe : public Pipe {
public:
    ResolveBlastPipe() {
        set_empty_block_set();
        set_empty_other();
        add(new AddSequences, "target=other");
        add(new AddBlocks, "target=other");
        add(new ResolveBlast);
        add(new CheckNoOverlaps);
        add(new UniqueNames);
        add(new Output);
    }
};

int main(int argc, char** argv) {
    return process(argc, argv, new ResolveBlastPipe,
                   "Add blast blocks, resolve overlaps");
}


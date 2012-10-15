/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <fstream>
#include <vector>

#include "Sequence.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "BlockSet.hpp"
#include "AddSequences.hpp"
#include "AnchorFinder.hpp"
#include "Exception.hpp"
#include "Output.hpp"
#include "po.hpp"

using namespace bloomrepeats;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    add_general_options(desc);
    AddSequences adder;
    adder.add_options(desc);
    AnchorFinder anchor_finder;
    po::positional_options_description pod;
    pod.add("in-seqs", -1);
    Output output;
    output.add_options(desc);
    anchor_finder.add_options(desc);
    po::variables_map vm;
    int error = read_options(argc, argv, vm, desc, pod);
    if (error) {
        return error;
    }
    try {
        anchor_finder.apply_options(vm);
    } catch (Exception& e) {
        std::cerr << argv[0] << ": error while setting up anchor finder: "
                  << std::endl << "  " << e.what() << std::endl;
        return 255;
    }
    try {
        adder.apply_options(vm);
        output.apply_options(vm);
    } catch (Exception& e) {
        std::cerr << argv[0] << ": " << e.what() << std::endl;
        return 255;
    }
    BlockSetPtr block_set = boost::make_shared<BlockSet>();
    adder.apply(block_set);
    anchor_finder.set_block_set(block_set);
    anchor_finder.run();
    output.apply(block_set);
}


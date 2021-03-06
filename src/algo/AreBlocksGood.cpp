/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>

#include "AreBlocksGood.hpp"
#include "SizeLimits.hpp"
#include "Rest.hpp"
#include "MoveGaps.hpp"
#include "CutGaps.hpp"
#include "Filter.hpp"
#include "BlockSet.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "FragmentCollection.hpp"
#include "Union.hpp"
#include "UniqueNames.hpp"
#include "block_stat.hpp"
#include "boundaries.hpp"
#include "hit.hpp"
#include "block_hash.hpp"
#include "throw_assert.hpp"
#include "global.hpp"

namespace npge {

AreBlocksGood::AreBlocksGood():
    file_writer_(this, "out-is-pangenome", "Output file with verdict") {
    add_size_limits_options(this);
    add_opt("respect-minor", "Do not check length and "
            "alignment of minor blocks", true);
    move_gaps_ = new MoveGaps();
    move_gaps_->set_parent(this);
    cut_gaps_ = new CutGaps();
    cut_gaps_->set_parent(this);
    filter_ = new Filter;
    filter_->set_parent(this);
    declare_bs("target", "Target blockset to be tested");
}

std::ostream& AreBlocksGood::get_out() const {
    return file_writer_.output();
}

bool AreBlocksGood::are_blocks_good() const {
    TimeIncrementer ti(this);
    bool good = true;
    UniqueNames un;
    Rest r(block_set());
    r.run();
    std::ostream& out = get_out();
    if (!r.block_set()->empty()) {
        good = false;
        out << "Sequences must be covered entirely by blocks. ";
        out << "There are " << r.block_set()->size()
            << " uncovered regions." << std::endl;
    }
    Strings alignmentless_blocks;
    Strings bad_blocks;
    Strings bad_cut_gaps_blocks;
    Strings bad_move_gaps_blocks;
    Strings overlaps_blocks;
    Strings self_overlaps_blocks;
    Strings neighbour_unique;
    int min_fragment_length;
    min_fragment_length = opt_value("min-fragment").as<int>();
    Decimal min_identity;
    min_identity = opt_value("min-identity").as<Decimal>();
    int respect_minor = opt_value("respect-minor").as<bool>();
    SetFc fc;
    fc.set_cycles_allowed(false);
    BOOST_FOREACH (Block* b, *block_set()) {
        bool minor = !b->name().empty() && b->name()[0] == 'm';
        bool m = respect_minor && minor;
        AlignmentStat al_stat;
        make_stat(al_stat, b);
        if (fc.block_has_overlap(b)) {
            overlaps_blocks.push_back(b->name());
            if (has_self_overlaps(b)) {
                self_overlaps_blocks.push_back(b->name());
            }
        }
        fc.add_block(b);
        if (m) {
            // minor block
        } else if (b->size() != 1) {
            if (al_stat.alignment_rows() != b->size()) {
                alignmentless_blocks.push_back(b->name());
            } else {
                if (!filter_->is_good_block(b)) {
                    bad_blocks.push_back(b->name());
                }
                boost::shared_ptr<Block> copy(b->clone());
                if (move_gaps_->move_gaps(copy.get())) {
                    bad_move_gaps_blocks.push_back(b->name());
                }
                boost::shared_ptr<Block> copy2(b->clone());
                if (cut_gaps_->cut_gaps(copy2.get())) {
                    bad_cut_gaps_blocks.push_back(b->name());
                }
            }
        }
    }
    // find subsequent unique
    BOOST_FOREACH (Block* b, *block_set()) {
        if (b->size() == 1) {
            Fragment* f = b->front();
            for (int ori = -1; ori <= 1; ori += 2) {
                Fragment* neighbour = fc.neighbor(f, ori);
                if (neighbour && neighbour->block() &&
                        neighbour->block()->size() == 1) {
                    neighbour_unique.push_back(f->id());
                }
            }
        }
    }
    if (!alignmentless_blocks.empty()) {
        good = false;
        out << "Following blocks do not have alignment: "
            << boost::algorithm::join(alignmentless_blocks, " ")
            << ".\n\n";
    }
    if (!bad_blocks.empty()) {
        good = false;
        out << "Following blocks are bad: "
            << boost::algorithm::join(bad_blocks, " ")
            << ".\n\n";
    }
    if (!bad_move_gaps_blocks.empty()) {
        good = false;
        out << "Following blocks have short 'tails' in alignment: "
            << boost::algorithm::join(bad_move_gaps_blocks, " ")
            << ".\n\n";
    }
    if (!bad_cut_gaps_blocks.empty()) {
        good = false;
        out << "Following blocks have end gaps in alignment: "
            << boost::algorithm::join(bad_cut_gaps_blocks, " ")
            << ".\n\n";
    }
    if (!neighbour_unique.empty()) {
        good = false;
        out << "Following unique fragments have unique neighbours: "
            << boost::algorithm::join(neighbour_unique, " ")
            << ".\n\n";
    }
    if (!overlaps_blocks.empty()) {
        good = false;
        out << "Following blocks have fragments overlapping neighbours: "
            << boost::algorithm::join(overlaps_blocks, " ")
            << ".\n\n";
    }
    if (!self_overlaps_blocks.empty()) {
        good = false;
        out << "Following blocks have self overlapping fragments: "
            << boost::algorithm::join(self_overlaps_blocks, " ")
            << ".\n\n";
    }
    return good;
}

void AreBlocksGood::run_impl() const {
    std::ostream& out = file_writer_.output();
    if (are_blocks_good()) {
        out << "[good blocks]" << std::endl;
    } else {
        out << "[not good blocks]" << std::endl;
    }
}

const char* AreBlocksGood::name_impl() const {
    return "Print if all blocks in blockset are good";
}

}


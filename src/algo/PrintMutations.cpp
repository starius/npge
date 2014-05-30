/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <string>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include "PrintMutations.hpp"
#include "Block.hpp"
#include "Fragment.hpp"
#include "throw_assert.hpp"
#include "global.hpp"

namespace npge {

PrintMutations::PrintMutations() {
    declare_bs("target", "Target blockset");
}

void PrintMutations::find_mutations(const Block* block,
                                    const MutationHandler& func) const {
    TimeIncrementer ti(this);
    std::string cons = block->consensus_string();
    int size = block->size();
    if (size == 1) {
        ASSERT_EQ(block->alignment_length(), cons.size());
    }
    BOOST_FOREACH (Fragment* f, *block) {
        int gaps = 0;
        std::string consensus;
        for (int pos = 0; pos < cons.size(); pos++) {
            char x = f->alignment_at(pos);
            if (size == 1) {
                ASSERT_EQ(x, cons[pos]);
            }
            if (x == '\0') {
                gaps += 1;
                consensus += cons[pos];
            }
            if (x != '\0' && gaps) {
                Mutation m;
                m.fragment = f;
                m.start = pos - gaps;
                m.stop = pos - 1;
                m.consensus = consensus;
                m.change = '-';
                func(m);
                gaps = 0;
                consensus = "";
            }
            if (x != '\0' && x != cons[pos]) {
                Mutation m;
                m.fragment = f;
                m.start = pos;
                m.stop = pos;
                m.consensus = cons[pos];
                m.change = x;
                func(m);
            }
        }
    }
}

static void print_change(std::ostream& o, const Mutation& m) {
    const Fragment* f = m.fragment;
    const Block* block = f->block();
    o << block->name() << '\t' << f->id() << '\t';
    o << m.start << '\t' << m.stop << '\t';
    o << m.consensus << '\t' << m.change << '\n';
}

void PrintMutations::print_block(std::ostream& o, Block* block) const {
    TimeIncrementer ti(this);
    find_mutations(block, boost::bind(print_change, boost::ref(o), _1));
}

void PrintMutations::print_header(std::ostream& o) const {
    o << "block" << '\t' << "fragment"
      << '\t' << "start" << '\t' << "stop"
      << '\t' << "consensus" << '\t' << "change" << '\n';
}

const char* PrintMutations::name_impl() const {
    return "Find all mutations in block";
}

}


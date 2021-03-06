/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <set>
#include <boost/foreach.hpp>

#include "block_stat.hpp"
#include "Block.hpp"
#include "Fragment.hpp"
#include "Sequence.hpp"
#include "BlockSet.hpp"
#include "boundaries.hpp"
#include "char_to_size.hpp"
#include "throw_assert.hpp"

namespace npge {

struct AlignmentStat::Impl {
    Impl():
        ident_nogap_(0),
        ident_gap_(0),
        noident_nogap_(0),
        noident_gap_(0),
        pure_gap_(0),
        total_(0),
        spreading_(0),
        alignment_rows_(0),
        min_fragment_length_(0) {
        memset(&atgc_, 0, LETTERS_NUMBER * sizeof(int));
    }

    int ident_nogap_;
    int ident_gap_;
    int noident_nogap_;
    int noident_gap_;
    int pure_gap_;
    int total_;
    Decimal spreading_;
    int alignment_rows_;
    int min_fragment_length_;
    int atgc_[LETTERS_NUMBER];
};

AlignmentStat::AlignmentStat() {
    impl_ = new Impl;
}

AlignmentStat::~AlignmentStat() {
    delete impl_;
    impl_ = 0;
}

int AlignmentStat::ident_nogap() const {
    return impl_->ident_nogap_;
}

int AlignmentStat::ident_gap() const {
    return impl_->ident_gap_;
}

int AlignmentStat::noident_nogap() const {
    return impl_->noident_nogap_;
}

int AlignmentStat::noident_gap() const {
    return impl_->noident_gap_;
}

int AlignmentStat::pure_gap() const {
    return impl_->pure_gap_;
}

int AlignmentStat::total() const {
    return impl_->total_;
}

Decimal AlignmentStat::spreading() const {
    return impl_->spreading_;
}

int AlignmentStat::alignment_rows() const {
    return impl_->alignment_rows_;
}

int AlignmentStat::min_fragment_length() const {
    return impl_->min_fragment_length_;
}

int AlignmentStat::letter_count(char letter) const {
    size_t letter_index = char_to_size(letter);
    if (letter_index < LETTERS_NUMBER) {
        return impl_->atgc_[letter_index];
    }
    return 0;
}

Decimal AlignmentStat::gc() const {
    Decimal gc = letter_count('G') + letter_count('C');
    Decimal at = letter_count('A') + letter_count('T');
    Decimal total = gc + at;
    return (total > 0) ? (gc / (gc + at)) : Decimal("-1");
}

// TODO rename Boundaries to smth
typedef Boundaries Integers;

void make_stat(AlignmentStat& stat, const Block* block, int start, int stop) {
    int alignment_length = block->alignment_length();
    if (stop == -1) {
        stop = alignment_length - 1;
    }
    stat.impl_->total_ = stop - start + 1;
    for (int pos = start; pos <= stop; pos++) {
        bool ident, gap, pure_gap;
        test_column(block, pos, ident, gap, pure_gap, stat.impl_->atgc_);
        if (!pure_gap) {
            if (ident && !gap) {
                stat.impl_->ident_nogap_ += 1;
            } else if (ident && gap) {
                stat.impl_->ident_gap_ += 1;
            } else if (!ident && !gap) {
                stat.impl_->noident_nogap_ += 1;
            } else if (!ident && gap) {
                stat.impl_->noident_gap_ += 1;
            }
        } else {
            stat.impl_->pure_gap_ += 1;
        }
    }
    Integers lengths;
    stat.impl_->alignment_rows_ = 0;
    BOOST_FOREACH (Fragment* f, *block) {
        lengths.push_back(f->length());
        if (f->row()) {
            stat.impl_->alignment_rows_ += 1;
        }
    }
    if (!lengths.empty()) {
        int max_length = *std::max_element(lengths.begin(), lengths.end());
        int min_length = *std::min_element(lengths.begin(), lengths.end());
        int avg_length = avg_element(lengths);
        if (avg_length == 0) {
            stat.impl_->spreading_ = 0;
        } else {
            Decimal range(max_length - min_length);
            stat.impl_->spreading_ = range / avg_length;
        }
        stat.impl_->min_fragment_length_ = min_length;
    }
}

bool is_ident_nogap(const Block* block, int column) {
    char seen_letter = 0;
    BOOST_FOREACH (Fragment* f, *block) {
        char c = f->alignment_at(column);
        if (c == 0) {
            return false;
        } else if (seen_letter == 0) {
            seen_letter = c;
        } else if (c != seen_letter) {
            return false;
        }
    }
    return true;
}

void test_column(const Block* block, int column,
                 bool& ident, bool& gap) {
    char seen_letter = 0;
    ident = true;
    gap = false;
    BOOST_FOREACH (Fragment* f, *block) {
        char c = f->alignment_at(column);
        if (c == 0) {
            gap = true;
        } else if (seen_letter == 0) {
            seen_letter = c;
        } else if (c != seen_letter) {
            ident = false;
        }
    }
}

void test_column(const Block* block, int column,
                 bool& ident, bool& gap, bool& pure_gap, int* atgc) {
    char seen_letter = 0;
    ident = true;
    gap = false;
    BOOST_FOREACH (Fragment* f, *block) {
        char c = f->alignment_at(column);
        if (c == 0) {
            gap = true;
        } else if (seen_letter == 0) {
            seen_letter = c;
        } else if (c != seen_letter) {
            ident = false;
        }
        if (c != 0) {
            size_t letter_index = char_to_size(c);
            if (letter_index < LETTERS_NUMBER) {
                atgc[letter_index] += 1;
            }
        }
    }
    pure_gap = !bool(seen_letter);
}

Decimal block_identity(const AlignmentStat& stat) {
    return block_identity(stat.ident_nogap(), stat.ident_gap(),
                          stat.noident_nogap(),
                          stat.noident_gap());
}

Decimal block_identity(int ident_nogap, int ident_gap,
                       int noident_nogap, int noident_gap) {
    Decimal accepted = Decimal(ident_nogap);
    accepted += Decimal(ident_gap) / 2;
    int total = ident_nogap + noident_nogap;
    total += ident_gap + noident_gap;
    return (total > 0) ? (accepted / total) : Decimal(0);
}

Decimal strict_block_identity(int ident_nogap, int ident_gap,
                              int noident_nogap,
                              int noident_gap) {
    int ident = ident_nogap + ident_gap;
    int noident = noident_nogap + noident_gap;
    int total = ident + noident;
    return (total > 0) ? (Decimal(ident_nogap) / total) : 0;
}

bool is_diagnostic(int col,
                   const Fragments& clade,
                   const Fragments& other) {
    ASSERT_GTE(clade.size(), 1);
    ASSERT_GTE(other.size(), 1);
    char clade_first = clade[0]->alignment_at(col);
    BOOST_FOREACH (Fragment* f, clade) {
        if (f->alignment_at(col) != clade_first) {
            return false;
        }
    }
    BOOST_FOREACH (Fragment* f, other) {
        if (f->alignment_at(col) == clade_first) {
            return false;
        }
    }
    return true;
}

Strings genomes_list(BlockSetPtr bs) {
    std::set<std::string> genomes;
    BOOST_FOREACH (SequencePtr seq, bs->seqs()) {
        genomes.insert(seq->genome());
    }
    Strings genomes_v(genomes.begin(), genomes.end());
    return genomes_v;
}

}


/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <set>
#include <boost/foreach.hpp>
#include <boost/cast.hpp>
#include <boost/scoped_ptr.hpp>

#include "SplitRepeats.hpp"
#include "PrintTree.hpp"
#include "tree.hpp"
#include "block_stat.hpp"
#include "char_to_size.hpp"
#include "Block.hpp"
#include "BlockSet.hpp"
#include "Fragment.hpp"
#include "Sequence.hpp"
#include "throw_assert.hpp"
#include "to_s.hpp"
#include "config.hpp"

namespace bloomrepeats {

SplitRepeats::SplitRepeats():
    BlocksJobs("other") {
    tree_ = new PrintTree;
    tree_->set_parent(this);
    add_opt("min-mutations", "Min number of mutations in "
            "candidate block to be splited",
            SPLIT_REPEATS_MIN_MUTATIONS);
    add_opt("min-diagnostic-mutations",
            "Min number of diagnostic mutations in "
            "part of block splitted out",
            SPLIT_REPEATS_MIN_DIAGNOSTIC_MUTATIONS);
    declare_bs("target", "Destination for weak blocks");
    declare_bs("other", "Input blocks (const)");
}

class SplitRepeatsData : public ThreadData {
public:
    Blocks blocks_to_insert;
};

ThreadData* SplitRepeats::before_thread_impl() const {
    return new SplitRepeatsData;
}

typedef std::set<std::string> StringSet;

static bool has_repeats(const Fragments& fragments) {
    StringSet genomes;
    BOOST_FOREACH (Fragment* f, fragments) {
        BOOST_ASSERT(f->seq());
        std::string genome = f->seq()->genome();
        if (genomes.find(genome) != genomes.end()) {
            return true;
        }
        genomes.insert(genome);
    }
    return false;
}

static void clade_to_fragments(Fragments& dst, TreeNode* clade) {
    Leafs leafs;
    clade->all_leafs(leafs);
    BOOST_FOREACH (LeafNode* leaf, leafs) {
        FragmentLeaf* fl;
        fl = boost::polymorphic_downcast<FragmentLeaf*>(leaf);
        Fragment* f = const_cast<Fragment*>(fl->fragment());
        dst.push_back(f);
    }
}

static void substract(Fragments& dst, const Fragments& all,
                      const Fragments& clade) {
    std::set<Fragment*> clade_set((clade.begin()), clade.end());
    BOOST_FOREACH (Fragment* f, all) {
        if (clade_set.find(f) == clade_set.end()) {
            dst.push_back(f);
        }
    }
}

static void find_repeated(StringSet& repeated,
                          const Fragments& all) {
    StringSet genomes;
    BOOST_FOREACH (Fragment* f, all) {
        BOOST_ASSERT(f->seq());
        std::string genome = f->seq()->genome();
        if (genomes.find(genome) != genomes.end()) {
            repeated.insert(genome);
        } else {
            genomes.insert(genome);
        }
    }
}

typedef std::vector<int> Ints;

static void find_mutations(Ints& mutations, const Block* block) {
    int l = block->alignment_length();
    for (int col = 0; col < l; col++) {
        bool ident, gap, pure_gap;
        int atgc[LETTERS_NUMBER];
        test_column(block, col, ident, gap, pure_gap, atgc);
        if (!ident || gap) {
            mutations.push_back(col);
        }
    }
}

static bool is_diagnostic(int col,
                          const Fragments& clade,
                          const Fragments& other) {
    BOOST_ASSERT(clade.size() >= 2);
    BOOST_ASSERT(other.size() >= 2);
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

static bool test_clade(const Fragments& clade,
                       const Fragments& all,
                       const StringSet& repeated,
                       const Ints& mutations,
                       int min_diagnostic) {
    if (clade.size() < 2 || clade.size() > all.size() - 2) {
        return false;
    }
    if (has_repeats(clade)) {
        return false;
    }
    bool genomes_with_repeats = false;
    BOOST_FOREACH (Fragment* f, clade) {
        BOOST_ASSERT(f->seq());
        std::string genome = f->seq()->genome();
        if (repeated.find(genome) != repeated.end()) {
            genomes_with_repeats = true;
        }
    }
    if (!genomes_with_repeats) {
        // no repeats - bad clade
        return false;
    }
    Fragments other;
    substract(other, all, clade);
    int diagnostic = 0;
    BOOST_FOREACH (int mutation, mutations) {
        if (is_diagnostic(mutation, clade, other)) {
            diagnostic += 1;
            if (diagnostic >= min_diagnostic) {
                break;
            }
        }
    }
    if (diagnostic < min_diagnostic) {
        return false;
    }
    return true;
}

typedef std::vector<Fragments> Clades;

struct CladeCmpRev {
    bool operator()(const Fragments& a, const Fragments& b) const {
        return b.size() < a.size();
    }
};

void SplitRepeats::process_block_impl(Block* block,
                                      ThreadData* data) const {
    int min_mutations = opt_value("min-mutations").as<int>();
    AlignmentStat stat;
    make_stat(stat, block);
    int mutations = stat.ident_gap() + stat.noident_nogap() +
                    stat.noident_gap();
    if (mutations < min_mutations) {
        // too few mutations
        return;
    }
    Ints mutcols;
    find_mutations(mutcols, block);
    int md = opt_value("min-diagnostic-mutations").as<int>();
    boost::scoped_ptr<TreeNode> tree(tree_->make_tree(block, "nj"));
    Nodes clades;
    tree->all_descendants(clades);
    Fragments all_ff((block->begin()), block->end());
    StringSet repeated;
    find_repeated(repeated, all_ff);
    Clades good_clades;
    BOOST_FOREACH (TreeNode* clade, clades) {
        Fragments clade_ff;
        clade_to_fragments(clade_ff, clade);
        if (test_clade(clade_ff, all_ff, repeated, mutcols, md)) {
            good_clades.push_back(clade_ff);
        }
        Fragments add_ff;
        substract(add_ff, all_ff, clade_ff);
        if (test_clade(add_ff, all_ff, repeated, mutcols, md)) {
            good_clades.push_back(add_ff);
        }
    }
    std::sort(good_clades.begin(), good_clades.end(),
              CladeCmpRev());
    SplitRepeatsData* d;
    d = boost::polymorphic_downcast<SplitRepeatsData*>(data);
    Blocks& new_blocks = d->blocks_to_insert;
    std::set<Fragment*> used_ff;
    int n = 0;
    BOOST_FOREACH (const Fragments& clade_ff, good_clades) {
        BOOST_ASSERT(test_clade(clade_ff, all_ff,
                                repeated, mutcols, md));
        bool used = false;
        BOOST_FOREACH (Fragment* f, clade_ff) {
            if (used_ff.find(f) != used_ff.end()) {
                used = true;
                break;
            }
        }
        if (!used) {
            n += 1;
            Block* new_block = new Block;
            new_blocks.push_back(new_block);
            new_block->set_weak(true);
            new_block->set_name(block->name() + "_" + TO_S(n));
            BOOST_FOREACH (Fragment* f, clade_ff) {
                BOOST_ASSERT(used_ff.find(f) == used_ff.end());
                used_ff.insert(f);
                new_block->insert(f);
            }
        }
    }
}

void SplitRepeats::after_thread_impl(ThreadData* data) const {
    BlockSet& target = *block_set();
    SplitRepeatsData* d;
    d = boost::polymorphic_downcast<SplitRepeatsData*>(data);
    Blocks& new_blocks = d->blocks_to_insert;
    BOOST_FOREACH (Block* new_block, new_blocks) {
        target.insert(new_block);
    }
}

const char* SplitRepeats::name_impl() const {
    return "Find splittable blocks-repeats and split them";
}

}

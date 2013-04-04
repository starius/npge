/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/foreach.hpp>

#include "CutGaps.hpp"
#include "Block.hpp"
#include "Fragment.hpp"
#include "AlignmentRow.hpp"
#include "throw_assert.hpp"
#include "Exception.hpp"

namespace bloomrepeats {

CutGaps::CutGaps(Mode mode):
    mode_(mode)
{ }

void CutGaps::add_options_impl(po::options_description& desc) const {
    RowStorage::add_options_impl(desc);
    std::string mode_str = (mode() == STRICT) ? "strict" : "permissive";
    add_unique_options(desc)
    ("cut-gaps-mode", po::value<std::string>()->default_value(mode_str),
     "mode of cutting gaps ('strict', 'permissive')");
}

void CutGaps::apply_options_impl(const po::variables_map& vm) {
    RowStorage::apply_options_impl(vm);
    if (vm.count("cut-gaps-mode")) {
        std::string mode_str = vm["cut-gaps-mode"].as<std::string>();
        if (mode_str == "strict") {
            set_mode(STRICT);
        } else if (mode_str == "permissive") {
            set_mode(PERMISSIVE);
        } else {
            throw Exception("Wrong cut-gaps-mode: " + mode_str);
        }
    }
}

static void slice_fragment(Fragment* f, int al_from, int al_to, RowType type) {
    AlignmentRow* old_row = f->row();
    BOOST_ASSERT(old_row);
    int fr_from = -1;
    for (int i = al_from; i <= al_to; i++) {
        fr_from = old_row->map_to_fragment(i);
        if (fr_from != -1) {
            break;
        }
    }
    if (fr_from == -1) {
        delete f;
        return;
    }
    int fr_to = -1;
    for (int i = al_to; i >= al_from; i--) {
        fr_to = old_row->map_to_fragment(i);
        if (fr_to != -1) {
            break;
        }
    }
    BOOST_ASSERT(fr_to != -1);
    int length = al_to - al_from + 1;
    std::stringstream ss;
    f->print_contents(ss, '-', /* line */ 0);
    std::string data = ss.str();
    std::string new_data = data.substr(al_from, length);
    AlignmentRow* new_row = AlignmentRow::new_row(type);
    new_row->grow(new_data);
    f->set_row(new_row);
    size_t begin = f->begin_pos() + fr_from * f->ori();
    size_t last = f->begin_pos() + fr_to * f->ori();
    f->set_begin_pos(begin);
    f->set_last_pos(last);
}

static void find_boundaries_strict(const Block* block, int& from, int& to) {
    int length = block->alignment_length();
    BOOST_FOREACH (Fragment* f, *block) {
        AlignmentRow* row = f->row();
        BOOST_ASSERT_MSG(row, ("No alignment row is set, fragment " +
                               f->id()).c_str());
        BOOST_ASSERT_MSG(row->length() == length,
                         ("Length of row of fragment " + f->id() + " (" +
                          boost::lexical_cast<std::string>(f->row()->length()) +
                          ") differs from block alignment length (" +
                          boost::lexical_cast<std::string>(length)).c_str());
    }
    from = 0;
    to = length - 1;
    for (; from <= to; from++) {
        bool gapless_column = true;
        BOOST_FOREACH (Fragment* f, *block) {
            AlignmentRow* row = f->row();
            if (row->map_to_fragment(from) == -1) {
                gapless_column = false;
                break;
            }
        }
        if (gapless_column) {
            break;
        }
    }
    for (; to >= from; to--) {
        bool gapless_column = true;
        BOOST_FOREACH (Fragment* f, *block) {
            AlignmentRow* row = f->row();
            if (row->map_to_fragment(to) == -1) {
                gapless_column = false;
                break;
            }
        }
        if (gapless_column) {
            break;
        }
    }
}

static void find_boundaries_permissive(const Block* block, int& from, int& to) {
    int length = block->alignment_length();
    from = 0;
    to = length - 1;
    BOOST_FOREACH (Fragment* f, *block) {
        AlignmentRow* row = f->row();
        BOOST_ASSERT_MSG(row, ("No alignment row is set, fragment " +
                               f->id()).c_str());
        BOOST_ASSERT_MSG(row->length() == length,
                         ("Length of row of fragment " + f->id() + " (" +
                          boost::lexical_cast<std::string>(f->row()->length()) +
                          ") differs from block alignment length (" +
                          boost::lexical_cast<std::string>(length)).c_str());
        for (int ori = -1; ori <= 1; ori += 2) {
            int begin = (ori == 1) ? 0 : length - 1;
            int i;
            for (i = 0; i < length; i++) {
                int al_pos = begin + i * ori;
                if (row->map_to_fragment(al_pos) != -1) {
                    if (ori == 1 && al_pos > from) {
                        from = al_pos;
                    } else if (ori == -1 && al_pos < to) {
                        to = al_pos;
                    }
                    break;
                }
            }
        }
    }
}

bool CutGaps::apply_to_block_impl(Block* block) const {
    bool result = false;
    int length = block->alignment_length();
    int from, to;
    if (mode() == STRICT) {
        find_boundaries_strict(block, from, to);
    } else if (mode() == PERMISSIVE) {
        find_boundaries_permissive(block, from, to);
    }
    if (from != 0 || to != length - 1) {
        result = true;
        if (to < from) {
            block->clear();
        } else {
            std::vector<Fragment*> fragments(block->begin(), block->end());
            BOOST_FOREACH (Fragment* f, fragments) {
                slice_fragment(f, from, to, row_type());
            }
        }
    }
    return result;
}

const char* CutGaps::name_impl() const {
    return "Cut terminal gaps";
}

}


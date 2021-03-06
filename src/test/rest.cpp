/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Rest.hpp"
#include "Sequence.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "BlockSet.hpp"
#include "Filter.hpp"

BOOST_AUTO_TEST_CASE (Rest_main) {
    using namespace npge;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tGGtccgagcgGAcggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tGGtccgagcggacggcc");
    Block* b1 = new Block();
    Fragment* f11 = new Fragment(s1, 1, 2);
    Fragment* f12 = new Fragment(s2, 1, 2);
    b1->insert(f11);
    b1->insert(f12);
    Block* b2 = new Block();
    Fragment* f21 = new Fragment(s1, 11, 12);
    b2->insert(f21);
    BlockSetPtr block_set = new_bs();
    block_set->insert(b1);
    block_set->insert(b2);
    BlockSetPtr rest = new_bs();
    Rest r(block_set);
    r.apply(rest);
    BOOST_CHECK(rest->size() == 5);
    Filter filter;
    filter.set_opt_value("min-block", 1);
    filter.set_opt_value("min-fragment", 2);
    filter.apply(rest);
    BOOST_CHECK(rest->size() == 3);
    filter.set_opt_value("min-fragment", 6);
    filter.apply(rest);
    BOOST_CHECK(rest->size() == 2);
    filter.set_opt_value("min-fragment", 8);
    filter.apply(rest);
    BOOST_CHECK(rest->size() == 2);
    filter.set_opt_value("min-fragment", 9);
    filter.apply(rest);
    BOOST_CHECK(rest->size() == 1);
}

BOOST_AUTO_TEST_CASE (Rest_self) {
    using namespace npge;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("AAA");
    Block* b1 = new Block();
    b1->insert(new Fragment(s1, 1, 1));
    BlockSetPtr block_set = new_bs();
    block_set->insert(b1);
    Rest r(block_set);
    r.apply(block_set);
    BOOST_CHECK(block_set->size() == 3);
}

BOOST_AUTO_TEST_CASE (Rest_of_empty) {
    using namespace npge;
    SequencePtr s1(new InMemorySequence("AAA"));
    BlockSetPtr block_set = new_bs();
    block_set->add_sequence(s1);
    Rest r(block_set);
    r.apply(block_set);
    BOOST_CHECK(block_set->size() == 1);
}


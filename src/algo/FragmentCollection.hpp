/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_FRAGMENT_COLLECTION_HPP_
#define BR_FRAGMENT_COLLECTION_HPP_

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "global.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "BlockSet.hpp"
#include "throw_assert.hpp"

namespace bloomrepeats {

class FragmentCompare {
public:
    bool operator()(const Fragment& a, const Fragment& b) const {
        return a < b;
    }

    bool operator()(const Fragment* a, const Fragment* b) const {
        return *a < *b;
    }
};

static FragmentCompare fc_;

template<typename F>
struct AssignFragment {
    void operator()(F& dest, Fragment* source) const;

    Fragment* operator()(const F& source) const;
};

template<>
struct AssignFragment<Fragment> {
    typedef Fragment F;

    void operator()(F& dest, Fragment* source) const {
        dest = *source;
    }

    Fragment* operator()(const F& source) const {
        return const_cast<Fragment*>(&source);
    }
};

template<>
struct AssignFragment<Fragment*> {
    typedef Fragment* F;

    void operator()(F& dest, Fragment* source) const {
        dest = source;
    }

    Fragment* operator()(const F& source) const {
        return const_cast<Fragment*>(source);
    }
};

template<typename F, typename C>
struct InsertFragment {
    void operator()(C& col, const F& fragment) const;
};

template<typename F>
struct InsertFragment<F, std::vector<F> > {
    typedef std::vector<F> C;

    void operator()(C& col, const F& f) const {
        col.push_back(f);
    }
};

template<typename F>
struct InsertFragment<F, std::set<F, FragmentCompare> > {
    typedef std::set<F, FragmentCompare> C;

    void operator()(C& col, const F& f) const {
        col.insert(f);
    }
};

template<typename C>
struct SortFragments {
    void operator()(C& col) const;
};

template<typename F>
struct SortFragments<std::vector<F> > {
    typedef std::vector<F> C;

    void operator()(C& col) const {
        std::sort(col.begin(), col.end(), fc_);
    }
};

template<typename F>
struct SortFragments<std::set<F, FragmentCompare> > {
    typedef std::set<F, FragmentCompare> C;

    void operator()(C& col) const {
        // set is already sorted
    }
};

template<typename F, typename C>
struct LowerBound {
    typename C::const_iterator
    operator()(const C& col, const F& fragment) const;
};

template<typename F>
struct LowerBound<F, std::vector<F> > {
    typedef std::vector<F> C;

    typename C::const_iterator
    operator()(const C& col, const F& fragment) const {
        return std::lower_bound(col.begin(), col.end(), fragment, fc_);
    }
};

template<typename F>
struct LowerBound<F, std::set<F, FragmentCompare> > {
    typedef std::set<F, FragmentCompare> C;

    typename C::const_iterator
    operator()(const C& col, const F& fragment) const {
        return col.lower_bound(fragment);
    }
};

/** Collection of fragments.
Template class.
First template parameter is type used to store fragments:
 - Fragment
 - Fragment*

Second template parameter is type used to store several fragments:
 - std::vector<F>
 - std::set<F, FragmentCompare>

Add fragments using add_fragment(), add_block() and add_bs().
Then call prepare().
After this you can use has_overlap() and find_overlaps() methods.

If std::set is used to store fragments, then call of prepare() is not needed.
You can add fragments and check overlaps in any order in this case.
*/
template<typename F, typename C>
class FragmentCollection {
public:
    /** Add a fragment to the collection */
    void add_fragment(Fragment* fragment) {
        F f;
        assigner_(f, fragment);
        Sequence* seq = fragment->seq();
        BOOST_ASSERT(seq);
        C& col = data_[seq];
        inserter_(col, f);
    }

    /** Add fragments of block to the collection */
    void add_block(Block* b) {
        BOOST_FOREACH (Fragment* f, *b) {
            add_fragment(f);
        }
    }

    /** Add fragments of block set to the collection */
    void add_bs(const BlockSet& bs) {
        BOOST_FOREACH (Block* b, bs) {
            add_block(b);
        }
    }

    /** Prepare collection for overlaps search.
    This step is not needed is std::set is used to store fragments.
    */
    void prepare() {
        BOOST_FOREACH (typename Seq2Fragments::value_type& seq_and_col, data_) {
            C& col = seq_and_col.second;
            sorter_(col);
        }
    }

    /** Clear collection */
    void clear() {
        data_.clear();
    }

    /** Return if the fragment overlaps any fragment from the collection */
    bool has_overlap(Fragment* fragment) const {
        F f;
        assigner_(f, fragment);
        Sequence* seq = fragment->seq();
        typename Seq2Fragments::const_iterator it = data_.find(seq);
        if (it == data_.end()) {
            return false;
        }
        const C& fragments = it->second;
        BOOST_ASSERT(!fragments.empty());
        typename C::const_iterator i2 = lower_bound_(fragments, f);
        if (i2 != fragments.end() &&
                assigner_(*i2)->common_positions(*fragment)) {
            return true;
        } else if (i2 != fragments.begin()) {
            i2--;
            if (assigner_(*i2)->common_positions(*fragment)) {
                return true;
            }
        }
        return false;
    }

    /** Find fragments overlapping with the fragment */
    void find_overlap_fragments(std::vector<Fragment*>& overlap_fragments,
                                Fragment* fragment) const {
        F f;
        assigner_(f, fragment);
        Sequence* seq = fragment->seq();
        typename Seq2Fragments::const_iterator it = data_.find(seq);
        if (it == data_.end()) {
            return;
        }
        const C& fragments = it->second;
        BOOST_ASSERT(!fragments.empty());
        typename C::const_iterator i2 = lower_bound_(fragments, f);
        typename C::const_iterator i2r = i2, i2l = i2;
        if (i2 != fragments.end() &&
                assigner_(*i2)->common_positions(*fragment)) {
            overlap_fragments.push_back(assigner_(*i2));
        }
        while (i2l != fragments.begin()) {
            i2l--;
            if (assigner_(*i2l)->common_positions(*fragment)) {
                overlap_fragments.push_back(assigner_(*i2l));
            }
        }
        while (i2r != fragments.end()) {
            i2r++;
            if (i2r != fragments.end() &&
                    assigner_(*i2r)->common_positions(*fragment)) {
                overlap_fragments.push_back(assigner_(*i2r));
            }
        }
    }

    /** Find overlaps between the fragment and fragments from the collection.
    Ori of fragment from collection is used.
    */
    void find_overlaps(std::vector<Fragment>& overlaps,
                       Fragment* fragment) const {
        std::vector<Fragment*> overlap_fragments;
        find_overlap_fragments(overlap_fragments, fragment);
        BOOST_FOREACH (Fragment* f, overlap_fragments) {
            BOOST_ASSERT(f->common_positions(*fragment));
            overlaps.push_back(f->common_fragment(*fragment));
            BOOST_ASSERT(overlaps.back().ori() == f->ori());
        }
    }

private:
    typedef std::map<Sequence*, C> Seq2Fragments;
    Seq2Fragments data_;
    AssignFragment<F> assigner_;
    InsertFragment<F, C> inserter_;
    SortFragments<C> sorter_;
    LowerBound<F, C> lower_bound_;
};

}

#endif

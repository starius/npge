/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_BLOCK_HPP_
#define NPGE_BLOCK_HPP_

#include <iosfwd>
#include <new>
#include <vector>
#include <string>
#include <boost/utility.hpp>

#include "global.hpp"
#include "Decimal.hpp"

namespace npge {

/** Container for fragments.
A block is aimed to keep related fragments together.

If block is not weak, it owns fragments added to it.
This means fragment->block() == this block for each fragment in block.
For weak block, this is optional. Weak block can own fragment as well.
Orphan fragments are fragments not owned by any block (fragment->block() == 0).
Orphan fragments added to block are parented even if block is weak.

If block owns a fragment, removing this fragment from block causes its deletion.
Make sure fragments being deleted are not accessed from weak blocks.
*/
class Block : boost::noncopyable {
public:
    /** Type of implementation container.
    Do not rely on ths type!
    To traverse all fragments, use BOOST_FOREACH (Fragment* f, block).
    For other operations use public members of Block.
    */
    typedef std::vector<Fragment*> Impl;

    /** Iterator */
    typedef Impl::iterator iterator;

    /** Constant iterator */
    typedef Impl::const_iterator const_iterator;

    /** Constructor.
    Set random name.
    */
    Block();

    /** Constructor */
    Block(const std::string& name);

    /** Destructor.
    Clear the block.
    */
    ~Block();

    /** Add fragment.
    If block is not weak or fragment is orphan,
    then fragment->block() is set to this block.
    */
    void insert(Fragment* fragment);

    /** Remove fragment.
    The fragment is deleted if fragment->block() == this.
    */
    void erase(Fragment* fragment);

    /** Remove fragment.
    The fragment is not deleted.
    */
    void detach(Fragment* fragment);

    /** Return the number of fragments in block */
    int size() const;

    /** Return if the block has no fragments */
    bool empty() const;

    /** Return if the block has the fragment */
    bool has(Fragment* fragment) const;

    /** Remove all fragments from the block.
    Removed fragments are deleted if fragment->block() == this.
    */
    void clear();

    /** Exchange values of two objects */
    void swap(Block& other);

    /** Get some fragment if any or an empty pointer */
    Fragment* front() const;

    /** Return iterator to beginning */
    iterator begin();

    /** Return constant iterator to beginning */
    const_iterator begin() const;

    /** Return iterator to end */
    iterator end();

    /** Return constant iterator to end */
    const_iterator end() const;

    /** Return length of alignment.
    If a fragment doesn't have alignment row attached,
    then length of the fragment is taken.
    */
    pos_t alignment_length() const;

    /** Return proportion of columns, composed of size() equal letters.
    If a fragment doesn't have alignment row attached,
    then it is taken as is.
    */
    Decimal identity() const;

    /** Return consensus letter for given position.
    For each column, the most frequent letter is written to consensus.
    If frequencies of several letters are equal, them some of them is written.
    For pure gap columns, value of argument 'gap' is written.
    */
    char consensus_char(pos_t pos, char gap = 'N') const;

    /** Write consensus to output stream.
    If front() has no row (in this case no other fragment must have row), then
    contents of longest fragment is written to stream.
    \see consensus_char
    */
    void consensus(std::ostream& o, char gap = 'N') const;

    /** Return consensus string */
    std::string consensus_string(char gap = 'N') const;

    /** Return if ori of fragments of two block correspond.
    0 means "no match", 1 means "match as is", -1 means "A match B.inverse()"
    */
    static int match(Block* one, Block* another);

    /** Inverse all fragments of this block.
    \see Fragment::inverse()
    */
    void inverse(bool inverse_row = true);

    /** Create new block as slice of this block.
    Alignment rows are sliced too if alignment = true.
    */
    Block* slice(pos_t start, pos_t stop,
                 bool alignment = true) const;

    /** Create copy of the block */
    Block* clone() const;

    /** Remove alignment rows of fragments */
    void remove_alignment();

    /** Return number of the fragment's positions, occupied by the block */
    pos_t common_positions(const Fragment& fragment) const;

    /** Move contents of other to this.
    Other is cleared.
    Duplicates are removed (\p other is \ref inverse "inversed" if needed).

    \warning Blocks must not be weak.
    */
    void merge(Block* other);

    /** Return name of block.
    By default, name is "00000000".
    */
    const std::string& name() const {
        return name_;
    }

    /** Set block name */
    void set_name(const std::string& name);

    /** Set random name */
    void set_random_name();

    /** Set name based on fragments.
    This name will be same for same sets on child fragments.
    */
    void set_name_from_fragments();

    /** Return if block is weak (does not own fragments) */
    bool weak() const {
        return weak_;
    }

    /** Set if block is weak (does not own fragments).
    If weak block turned to non-weak, then all its fragments are reparented
    to this block and previous blocks are marked as weak.
    */
    void set_weak(bool weak);

    /** Compare blocksets.
    This is implemented as comparison of hashes.
    */
    bool operator==(const Block& other) const;

private:
    Impl fragments_;
    std::string name_;
    bool weak_;
};

/** Streaming operator */
std::ostream& operator<<(std::ostream& o, const Block& block);

}

#endif


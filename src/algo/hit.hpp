/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_HIT_HPP_
#define NPGE_HIT_HPP_

#include "FragmentCollection.hpp"

namespace npge {

/** Return if hit is internal for blockset (s2f).
This means, each fragment of hit is a subfragment of one fragment
from s2f and size of corresponding block from blockset is
less than size of hit.
*/
bool is_internal_hit(const SetFc& s2f, const Block* hit,
                     bool allow_no_overlaps = false);

/** Return if block contain self-overlapping fragments */
bool has_self_overlaps(Block* block);

/** Shorten or remove self-overlapping fragments */
void fix_self_overlaps(Block* block);

}

#endif


/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2013 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "MakePrePangenome.hpp"
#include "AnchorFinder.hpp"
#include "FragmentsExpander.hpp"
#include "Filter.hpp"
#include "OverlapsResolver2.hpp"
#include "Align.hpp"
#include "Rest.hpp"
#include "Connector.hpp"
#include "to_s.hpp"

namespace npge {

MakePrePangenome::MakePrePangenome() {
    add(new AnchorFinder);
    add(new Connector);
    add(new FragmentsExpander,
        "--max-overlap:=$EXPANDER_MAX_OVERLAP");
    add(new Filter);
    add(new OverlapsResolver2, "target=target other=target");
    add(new Filter);
    add(new FragmentsExpander,
        "--max-overlap:=$EXPANDER_MAX_OVERLAP");
    add(new OverlapsResolver2, "target=target other=target");
    add(new Align);
    add(new Filter);
    add(new Rest, "other=target");
    declare_bs("target", "Target blockset with sequences "
               "on which pre-pangenome is built");
}

const char* MakePrePangenome::name_impl() const {
    return "Run anchor finder, expand blocks and resolve overlaps "
           "(deprecated)";
}

}


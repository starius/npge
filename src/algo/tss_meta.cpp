/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "boost-xtime.hpp"
#include <boost/thread/tss.hpp>

#include "tss_meta.hpp"
#include "Meta.hpp"

namespace npge {

static boost::thread_specific_ptr<Meta> tss_meta_;

Meta* tss_meta() {
    if (tss_meta_.get() == 0) {
        void* ptr = ::operator new(sizeof(Meta));
        Meta* meta = reinterpret_cast<Meta*>(ptr);
        tss_meta_.reset(meta);
        new(meta) Meta;
    }
    return tss_meta_.get();
}

AnyAs tss_go(const std::string& key, const AnyAs& dflt) {
    return tss_meta()->get_opt(key, dflt);
}

void delete_tss_meta() {
    tss_meta_.reset();
}

Meta* release_tss_meta() {
    return tss_meta_.release();
}

}


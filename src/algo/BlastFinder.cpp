/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include "BlastFinder.hpp"
#include "RawWrite.hpp"
#include "BlastRunner.hpp"
#include "ImportBlastHits.hpp"
#include "FileCopy.hpp"
#include "FileRemover.hpp"
#include "BlockSet.hpp"
#include "name_to_stream.hpp"

namespace npge {

BlastFinder::BlastFinder() {
    OptionGetter c = boost::bind(&BlastFinder::consensus, this);
    OptionGetter h = boost::bind(&BlastFinder::hits, this);
    RawWrite* cons = new RawWrite;
    add(cons);
    cons->fix_opt_value("dump-seq", true);
    cons->fix_opt_value("dump-block", false);
    cons->fix_opt_getter("file", c);
    BlastRunner* runner = new BlastRunner;
    add(runner);
    runner->fix_opt_getter("in-consensus", c);
    runner->fix_opt_getter("out-hits", h);
    ImportBlastHits* importer = new ImportBlastHits;
    add(importer);
    importer->point_bs("other=target", this);
    importer->fix_opt_getter("blast-hits", h);
    FileCopy* copy_c = new FileCopy;
    add(copy_c);
    copy_c->set_opt_prefix("blast-cons-");
    copy_c->fix_opt_getter("src", c);
    FileCopy* copy_h = new FileCopy;
    add(copy_h);
    copy_h->set_opt_prefix("blast-hits-");
    copy_h->fix_opt_getter("src", h);
    FileRemover* rm_c = new FileRemover;
    add(rm_c);
    rm_c->fix_opt_getter("filename", c);
    FileRemover* rm_h = new FileRemover;
    add(rm_h);
    rm_h->fix_opt_getter("filename", h);
    declare_bs("target", "Blockset in which hits are searched");
}

void BlastFinder::run_impl() const {
    if (!block_set()->seqs().empty()) {
        Pipe::run_impl();
    }
}

std::string BlastFinder::consensus() const {
    if (consensus_.empty()) {
        consensus_ = tmp_file();
    }
    return consensus_;
}

std::string BlastFinder::hits() const {
    if (hits_.empty()) {
        hits_ = tmp_file();
    }
    return hits_;
}

}


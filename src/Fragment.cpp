/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <ostream>

#include "Fragment.hpp"
#include "Sequence.hpp"
#include "complement.hpp"

namespace bloomrepeats {

Fragment::Fragment(SequencePtr seq, size_t min_pos, size_t max_pos, int ori):
    seq_(seq), min_pos_(min_pos), max_pos_(max_pos), ori_(ori)
{ }

BlockPtr Fragment::block() const {
    return block_.lock();
}

size_t Fragment::length() const {
    return max_pos() - min_pos() + 1;
}

size_t Fragment::begin_pos() const {
    return ori() == 1 ? min_pos() : max_pos();
}

const char* Fragment::begin() const {
    size_t l = length();
    return seq()->get(begin_pos(), l);
}

size_t Fragment::end_pos() const {
    return ori() == 1 ? max_pos() + 1 : min_pos() - 1;
}

const char* Fragment::end() const {
    return begin() + length() * ori();
}

void Fragment::inverse() {
    set_ori(ori() == 1 ? -1 : 1);
}

std::string Fragment::str() const {
    std::string result;
    if (ori() == 1) {
        result.assign(begin(), length());
    } else {
        result.reserve(length());
        for (const char* c = begin(); c != end(); c--) {
            result += complement(*c);
        }
    }
    return result;
}

char Fragment::expand() {
    char result = 0;
    if (ori() == 1 && max_pos() + 1 < seq()->size()) {
        result = *end();
        set_max_pos(max_pos() + 1);
    } else if (/* ori() == -1 && */ min_pos() > 0) {
        result = *end();
        set_min_pos(min_pos() - 1);
    }
    return result;
}

void Fragment::compress() {
    if (ori() == 1) {
        set_max_pos(max_pos() - 1);
    } else {
        set_min_pos(min_pos() + 1);
    }
}

bool Fragment::operator==(const Fragment& other) const {
    return min_pos() == other.min_pos() && max_pos() == other.max_pos() &&
           ori() == other.ori() && seq() == other.seq();
}

bool Fragment::operator!=(const Fragment& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& o, const Fragment& f) {
    for (const char* c = f.begin(); c != f.end(); c += f.ori()) {
        o << (f.ori() == 1 ? *c : complement(*c));
    }
    return o;
}

}


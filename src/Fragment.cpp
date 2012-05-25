/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <ostream>
#include <algorithm>
#include <boost/assert.hpp>

#include "Fragment.hpp"
#include "Sequence.hpp"
#include "complement.hpp"
#include "make_hash.hpp"

namespace bloomrepeats {

Fragment::Fragment(SequencePtr seq, size_t min_pos, size_t max_pos, int ori):
    seq_(seq), min_pos_(min_pos), max_pos_(max_pos), ori_(ori)
{ }

BlockPtr Fragment::block() const {
    return block_.lock();
}

FragmentPtr Fragment::prev() const {
    return prev_.lock();
}

FragmentPtr Fragment::next() const {
    return next_.lock();
}

FragmentPtr Fragment::neighbour(int ori) const {
    return ori == 1 ? next() : prev();
}

FragmentPtr Fragment::logical_neighbour(int ori) const {
    return neighbour(this->ori() * ori);
}

bool Fragment::is_neighbour(const Fragment& other) const {
    return prev().get() == &other || next().get() == &other;
}

FragmentPtr Fragment::another_neighbour(const Fragment& other) const {
    BOOST_ASSERT(is_neighbour(other));
    return prev().get() == &other ? next() : prev();
}

size_t Fragment::length() const {
    return max_pos() - min_pos() + 1;
}

size_t Fragment::begin_pos() const {
    return ori() == 1 ? min_pos() : max_pos();
}

void Fragment::set_begin_pos(size_t begin_pos) {
    if (ori() == 1) {
        set_min_pos(begin_pos);
    } else {
        set_max_pos(begin_pos);
    }
}

const char* Fragment::begin() const {
    size_t l = length();
    return seq()->get(begin_pos(), l);
}

size_t Fragment::last_pos() const {
    return ori() == 1 ? max_pos() : min_pos();
}

void Fragment::set_last_pos(size_t last_pos) {
    if (ori() == 1) {
        set_max_pos(last_pos);
    } else {
        set_min_pos(last_pos);
    }
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

std::string Fragment::substr(int min, int max) const {
    min = min < 0 ? length() + min : min;
    max = max < 0 ? length() + max : max;
    int l = max - min + 1;
    std::string result;
    if (ori() == 1) {
        result.assign(begin() + min, l);
    } else {
        result.reserve(l);
        for (const char* c = begin() - min; c >= begin() - max; c--) {
            result += complement(*c);
        }
    }
    return result;
}

size_t Fragment::hash() const {
    return make_hash(begin(), length(), ori());
}

void Fragment::shift_end(int shift) {
    if (ori() == 1) {
        set_max_pos(max_pos() + shift);
    } else { /* if (ori() == -1) */
        set_min_pos(min_pos() - shift);
    }
}

int Fragment::max_shift_end(bool overlap) const {
    int result = ori() == 1 ? seq()->size() - max_pos() - 1 : min_pos();
    if (overlap == false) {
        FragmentPtr neighbour = logical_neighbour(1);
        if (neighbour) {
            result = ori() == 1 ? neighbour->min_pos() - max_pos() - 1 :
                     min_pos() - neighbour->max_pos() - 1;
        }
    }
    return result;
}

bool Fragment::valid() const {
    return min_pos() <= max_pos() && max_pos() < seq()->size();
}

bool Fragment::operator==(const Fragment& other) const {
    return min_pos() == other.min_pos() && max_pos() == other.max_pos() &&
           ori() == other.ori() && seq() == other.seq();
}

bool Fragment::operator!=(const Fragment& other) const {
    return !(*this == other);
}

bool Fragment::operator<(const Fragment& other) const {
    return min_pos() < other.min_pos() ||
           (min_pos() == other.min_pos() &&
            (max_pos() < other.max_pos() ||
             (max_pos() == other.max_pos() &&
              ori() < other.ori())));
}

char Fragment::raw_at(int pos) const {
    char raw = *(begin() + ori() * pos);
    return ori() == 1 ? raw : complement(raw);
}

char Fragment::at(int pos) const {
    return raw_at(pos >= 0 ? pos : length() + pos);
}

void Fragment::connect(FragmentPtr first, FragmentPtr second) {
    BOOST_ASSERT(first);
    BOOST_ASSERT(second);
    if (first->next() != second) {
        if (first->next()) {
            first->next()->prev_.reset();
        }
        if (second->prev()) {
            second->prev()->next_.reset();
        }
    }
    first->next_ = second;
    second->prev_ = first;
}

void Fragment::connect(FragmentPtr first, FragmentPtr second, int ori) {
    if (ori == 1) {
        connect(first, second);
    } else {
        connect(second, first);
    }
}

void Fragment::rearrange_with(FragmentPtr other) {
    FragmentPtr this_prev = prev();
    FragmentPtr this_next = next();
    FragmentPtr other_prev = other->prev();
    FragmentPtr other_next = other->next();
    FragmentPtr this_ptr = shared_from_this();
    if (this_prev && this_prev != other) {
        connect(this_prev, other);
    }
    if (this_next && this_next != other) {
        connect(other, this_next);
    }
    if (other_prev && other_prev != this_ptr) {
        connect(other_prev, this_ptr);
    }
    if (other_next && other_next != this_ptr) {
        connect(this_ptr, other_next);
    }
    if (this_next == other) {
        connect(other, this_ptr);
    }
    if (other_next == this_ptr) {
        connect(this_ptr, other);
    }
}

void Fragment::find_place() {
    for (int ori = -1; ori <= 1; ori += 2) {
        while (FragmentPtr n = neighbour(ori)) {
            if ((ori == 1 && *n < *this) || (ori == -1 && *this < *n)) {
                rearrange_with(n);
            } else {
                break;
            }
        }
    }
}

bool Fragment::can_merge(FragmentPtr one, FragmentPtr another) {
    return one->seq() == another->seq() && one->ori() == another->ori() &&
           one->is_neighbour(*another);
}

FragmentPtr Fragment::merge(FragmentPtr one, FragmentPtr another) {
    BOOST_ASSERT(can_merge(one, another));
    if (another->next() == one) {
        std::swap(one, another);
    }
    FragmentPtr new_fragment = boost::make_shared<Fragment>(one->seq(),
                               std::min(one->min_pos(), another->min_pos()),
                               std::max(one->max_pos(), another->max_pos()),
                               one->ori());
    if (one->prev()) {
        connect(one->prev(), new_fragment);
    }
    if (another->next()) {
        connect(new_fragment, another->next());
    }
    return new_fragment;
}

void Fragment::disconnect() {
    if (next()) {
        next()->prev_.reset();
    }
    if (prev()) {
        prev()->next_.reset();
    }
    next_.reset();
    prev_.reset();
}

size_t Fragment::common_positions(const Fragment& other) {
    size_t result = 0;
    if (seq() == other.seq()) {
        size_t max_min = std::max(min_pos(), other.min_pos());
        size_t min_max = std::min(max_pos(), other.max_pos());
        if (max_min <= min_max) {
            result = min_max - max_min + 1;
        }
    }
    return result;
}

FragmentPtr Fragment::common_fragment(const Fragment& other) {
    FragmentPtr res;
    if (seq() == other.seq()) {
        size_t max_min = std::max(min_pos(), other.min_pos());
        size_t min_max = std::min(max_pos(), other.max_pos());
        if (max_min <= min_max) {
            res = boost::make_shared<Fragment>(seq(), max_min, min_max, ori());
            BOOST_ASSERT(res->length() == common_positions(other));
        }
    }
    return res;
}

Fragment::Diff Fragment::diff_to(const Fragment& other) const {
    BOOST_ASSERT(seq() == other.seq());
    Diff diff;
    diff.begin = ori() * (int(other.begin_pos()) - int(begin_pos()));
    diff.last = ori() * (int(other.last_pos()) - int(last_pos()));
    diff.ori = other.ori() == ori() ? 1 : -1;
    return diff;
}

void Fragment::patch(const Fragment::Diff& diff) {
    size_t new_begin = begin_pos() + ori() * diff.begin;
    size_t new_last = last_pos() + ori() * diff.last;
    set_ori(ori() * diff.ori);
    set_begin_pos(new_begin);
    set_last_pos(new_last);
}

void Fragment::apply_coords(const Fragment& other) {
    seq_ = other.seq();
    set_min_pos(other.min_pos());
    set_max_pos(other.max_pos());
    set_ori(other.ori());
}

void Fragment::exclude(const Fragment& other) {
    BOOST_ASSERT(seq() == other.seq());
    size_t max_min = std::max(min_pos(), other.min_pos());
    size_t min_max = std::min(max_pos(), other.max_pos());
    if (max_min <= min_max) {
        if (min_pos() < other.min_pos()) {
            set_max_pos(other.min_pos() - 1);
        } else if (max_pos() > other.max_pos()) {
            set_min_pos(other.max_pos() + 1);
        } else {
            size_t old_min = min_pos();
            set_min_pos(max_pos() + 1); // +1 for fragments of length=1
            set_max_pos(old_min);
            BOOST_ASSERT(!valid());
        }
    }
}

void Fragment::split(const Fragment& main_part, FragmentPtr& other_part) {
    BOOST_ASSERT(seq() == main_part.seq());
    Fragment other_fragment = *this;
    other_fragment.exclude(main_part);
    apply_coords(main_part);
    if (other_fragment.valid()) {
        other_part = boost::make_shared<Fragment>();
        other_part->apply_coords(other_fragment);
        if (this->next()) {
            connect(other_part, this->next());
        }
        connect(shared_from_this(), other_part);
        other_part->find_place();
    }
    this->find_place();
}

std::ostream& operator<<(std::ostream& o, const Fragment& f) {
    for (const char* c = f.begin(); c != f.end(); c += f.ori()) {
        o << (f.ori() == 1 ? *c : complement(*c));
    }
    return o;
}

}


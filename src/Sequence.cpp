/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <fstream>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/bind.hpp>
#include <boost/utility/binary.hpp>

#include "Sequence.hpp"
#include "Fragment.hpp"
#include "FastaReader.hpp"
#include "char_to_size.hpp"

namespace bloomrepeats {

Sequence::Sequence():
    size_(0)
{ }

void Sequence::make_first_fragment(Fragment& f, size_t fragment_size,
                                   int only_ori) const {
    f.set_min_pos(0 - 1);
    f.set_max_pos(fragment_size - 1 - 1);
    f.set_ori(only_ori ? : 1);
}

bool Sequence::next_fragment(Fragment& f) const {
    f.set_ori(-f.ori());
    if (f.ori() == -1) {
        f.set_min_pos(f.min_pos() + 1);
        f.set_max_pos(f.max_pos() + 1);
    }
    return f.max_pos() < size();
}

bool Sequence::next_fragment_keeping_ori(Fragment& f) const {
    f.set_min_pos(f.min_pos() + 1);
    f.set_max_pos(f.max_pos() + 1);
    return f.max_pos() < size();
}

void Sequence::to_atgc(std::string& data) {
    using namespace boost::algorithm;
    to_lower(data);
    data.erase(std::remove_if(data.begin(), data.end(),
                              !boost::bind<bool>(is_any_of("atgc"), _1)),
               data.end());
}

std::string Sequence::genome() const {
    using namespace boost::algorithm;
    std::vector<std::string> parts;
    split(parts, name(), is_any_of("&"));
    if (parts.size() == 3 && (parts[2] == "c" || parts[2] == "l")) {
        return parts[0];
    } else {
        return "";
    }
}

std::string Sequence::chromosome() const {
    using namespace boost::algorithm;
    std::vector<std::string> parts;
    split(parts, name(), is_any_of("&"));
    if (parts.size() == 3 && (parts[2] == "c" || parts[2] == "l")) {
        return parts[1];
    } else {
        return "";
    }
}

bool Sequence::circular() const {
    using namespace boost::algorithm;
    std::vector<std::string> parts;
    split(parts, name(), is_any_of("&"));
    if (parts.size() == 3 && (parts[2] == "c" || parts[2] == "l")) {
        return parts[2] == "c";
    } else {
        throw std::logic_error("Bad name to deduce "
                               "linear/circular: " + name());
    }
}

void Sequence::read_all_seqs(std::istream& input,
                             std::vector<SequencePtr>& seqs,
                             Sequence::StorageMode mode) {
    while (true) {
        SequencePtr seq;
        if (mode == COMPACT) {
            seq = SequencePtr(new CompactSequence(input));
        } else {
            // use this method by default
            seq = SequencePtr(new InMemorySequence(input));
        }
        if (seq->size() > 0) {
            seqs.push_back(seq);
        } else {
            break;
        }
    }
}

InMemorySequence::InMemorySequence(const std::string& filename, int) {
    std::ifstream file(filename.c_str());
    read_from_file(file);
}

InMemorySequence::InMemorySequence(std::istream& input) {
    read_from_file(input);
}

InMemorySequence::InMemorySequence(const std::string& data):
    data_(data) {
    to_atgc(data_);
    set_size(data_.size());
}

char InMemorySequence::char_at(size_t index) const {
    return data_[index];
}

template <typename F>
class SequenceFastaReader : public FastaReader {
public:
    SequenceFastaReader(Sequence& seq, std::istream& input, const F& f):
        FastaReader(input), seq_(seq), f_(f)
    { }

    void new_sequence(const std::string& name, const std::string& description) {
        seq_.set_name(name);
        seq_.set_description(name);
    }

    void grow_sequence(const std::string& data) {
        std::string line(data);
        Sequence::to_atgc(line);
        f_(line);
    }

private:
    Sequence& seq_;
    const F& f_;
};

template <typename F>
static void read_fasta(Sequence& seq, std::istream& input, const F& f) {
    SequenceFastaReader<F> reader(seq, input, f);
    reader.read_one_sequence();
}

void InMemorySequence::read_from_file(std::istream& input) {
    typedef std::string& (std::string::* StringMethod)(const std::string&);
    StringMethod append = &std::string::append;
    read_fasta(*this, input, boost::bind(append, &data_, _1));
    set_size(data_.size());
}

CompactSequence::CompactSequence(std::istream& input) {
    read_from_file(input);
}

CompactSequence::CompactSequence(const std::string& data) {
    std::string data_copy(data);
    to_atgc(data_copy);
    add_hunk(data_copy);
}

const size_t LAST_TWO_BITS = BOOST_BINARY(11);

char CompactSequence::char_at(size_t index) const {
    size_t s = (data_[byte_index(index)] >> shift(index)) & LAST_TWO_BITS;
    return size_to_char(s);
}

void CompactSequence::read_from_file(std::istream& input) {
    read_fasta(*this, input,
               boost::bind(&CompactSequence::add_hunk, this, _1));
}

void CompactSequence::add_hunk(const std::string& hunk) {
    size_t new_size = size() + hunk.size();
    if (byte_index(new_size - 1) >= data_.size()) {
        data_.resize(byte_index(new_size - 1) + 1);
    }
    for (size_t i = 0; i < hunk.size(); i++) {
        set_item(size() + i, hunk[i]);
    }
    set_size(new_size);
}

void CompactSequence::set_item(size_t index, char value) {
    data_[byte_index(index)] |= char_to_size(value) << shift(index);
}

size_t CompactSequence::byte_index(size_t index) const {
    return index / 4;
}

size_t CompactSequence::shift(size_t index) const {
    return 2 * (index % 4);
}

}


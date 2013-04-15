/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_KEY_VALUE_HPP_
#define BR_KEY_VALUE_HPP_

#include <string>

namespace bloomrepeats {

/** Extract value for given key from string like "key1=value1 key2=value2".
If given key is not found, return "".
*/
std::string extract_value(const std::string& values, const std::string& key);

}

#endif

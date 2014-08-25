/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_MODEL_LUA_HPP_
#define NPGE_MODEL_LUA_HPP_

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "global.hpp"

namespace luabind {

typedef npge::Fragments Fragments;

template <>
struct default_converter<Fragments> :
        native_converter_base<Fragments> {
    static int compute_score(lua_State* L, int index);
    Fragments from(lua_State* L, int index);
    void to(lua_State* L, const Fragments& strings);
};

template <>
struct default_converter<const Fragments&> :
        default_converter<Fragments> {
};

typedef npge::Blocks Blocks;

template <>
struct default_converter<Blocks> :
        native_converter_base<Blocks> {
    static int compute_score(lua_State* L, int index);
    Blocks from(lua_State* L, int index);
    void to(lua_State* L, const Blocks& strings);
};

template <>
struct default_converter<const Blocks&> :
        default_converter<Blocks> {
};

}

/** Initialize model/ members in Lua */
extern "C" int init_model_lua(lua_State* L);

#endif


//--------------------------------------------------------------------------
// Copyright (C) 2015-2015 Cisco and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------
// pp_raw_buffer_iface.cc author Joel Cornett <jocornet@cisco.com>

#include "pp_raw_buffer_iface.h"

#include "lua/lua_arg.h"
#include "lua/lua_stack.h"

// FIXIT-H: A lot of users keep references to this data.
//          Need to prevent Lua's garbage collection from destroying RawBuffer
//          while other C++ types are using the data (unbeknowest to Lua).
//          Add a container data type which hold ref counts to RawBuffer
//          and only frees when the ref count is zero.

static int init_from_string(lua_State* L)
{
    Lua::Arg arg(L);

    size_t len = 0;
    const char* s = arg.check_string(1, &len);
    size_t size = arg.opt_size(2, len);

    // instantiate and adjust size if necessary
    RawBufferIface.create(L, s, len).resize(size, '\0');

    return 1;
}

static int init_from_size(lua_State* L)
{
    Lua::Arg arg(L);

    size_t size = arg.opt_size(1);

    RawBufferIface.create(L, size, '\0');

    return 1;
}

static const luaL_Reg methods[] =
{
    {
        "new",
        [](lua_State* L)
        {
            Lua::Arg arg(L);

            if ( arg.is_string(1) )
                return init_from_string(L);

            return init_from_size(L);
        }
    },
    {
        "size",
        [](lua_State* L)
        {
            auto& self = RawBufferIface.get(L);
            lua_pushinteger(L, self.size());

            return 1;
        }
    },
    {
        "resize",
        [](lua_State* L)
        {
            Lua::Arg arg(L);

            auto& self = RawBufferIface.get(L, 1);
            size_t new_size = arg.check_size(2);
            
            self.resize(new_size, '\0');

            return 0;
        }
    },
    {
        "write",
        [](lua_State* L)
        {
            Lua::Arg arg(L);

            auto& self = RawBufferIface.get(L, 1);

            size_t len = 0;
            const char* s = arg.check_string(2, &len);
            size_t offset = arg.opt_size(3);

            size_t required = offset + len;
            if ( self.size() < required )
                self.resize(required, '\0');

            self.replace(offset, len, s);

            return 0;
        }
    },
    {
        "read",
        [](lua_State* L)
        {
            Lua::Arg arg(L);

            auto& self = RawBufferIface.get(L, 1);

            if ( arg.count > 2 )
            {
                size_t start = arg.check_size(2, self.size());
                size_t end = arg.check_size(3, start, self.size());
                lua_pushlstring(L, self.data() + start, end - start);
            }
            else
            {
                size_t end = arg.opt_size(2, self.size(), self.size());
                lua_pushlstring(L, self.data(), end);
            }

            return 1;
        }
    },
    { nullptr, nullptr }
};

static const luaL_Reg metamethods[] =
{
    {
        "__tostring",
        [](lua_State* L)
        {
            auto& self = RawBufferIface.get(L);
            lua_pushfstring(L, "%s@%p", RawBufferIface.name, &self);
            return 1;
        }
    },
    {
        "__gc",
        [](lua_State* L)
        {
            RawBufferIface.destroy(L);
            return 0;
        }
    },
    { nullptr, nullptr }
};

const struct Lua::TypeInterface<RawBuffer> RawBufferIface =
{
    "RawBuffer",
    methods,
    metamethods
};
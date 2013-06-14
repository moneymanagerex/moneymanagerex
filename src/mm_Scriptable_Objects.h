/*******************************************************
 Copyright (C) 2012 Lisheng Guan 

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#if !defined(MM_SCRIPTABLE_OBJECTS_H)
#define MM_SCRIPTABLE_OBJECTS_H

template <typename T>
class mm_Lua_Object
{
    typedef struct { T *pT; } User_Data;
public:
    typedef int (T::*mfp)(lua_State *L);
    typedef struct { const char *name; mfp mfunc; } Registration_Record;

    static void register_self(lua_State *L)
    {
        lua_newtable(L);
        int methods = lua_gettop(L);

        luaL_newmetatable(L, T::className);
        int metatable = lua_gettop(L);

        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushvalue(L, methods);
        set(L, LUA_REGISTRYINDEX, T::className);

        // hide metatable from Lua getmetatable()
        lua_pushvalue(L, methods);
        set(L, metatable, "__metatable");

        lua_pushvalue(L, methods);
        set(L, metatable, "__index");

        lua_pushcfunction(L, tostring_T);
        set(L, metatable, "__tostring");

        lua_pushcfunction(L, gc_T);
        set(L, metatable, "__gc");

        // fill method table with methods from class T
        for (Registration_Record *l = T::methods; l->name; l++)
        {
            lua_pushstring(L, l->name);
            lua_pushlightuserdata(L, (void*)l);
            lua_pushcclosure(L, thunk, 1);
            lua_settable(L, methods);
        }

        lua_pop(L, 2);  // drop metatable and method table

    }

    // push onto the Lua stack a userdata containing a pointer to T object
    static int push(lua_State *L, T *obj, bool gc=false)
    {
        if (!obj) { lua_pushnil(L); return 0; }
        luaL_getmetatable(L, T::className);  // lookup metatable in Lua registry
        if (lua_isnil(L, -1)) luaL_error(L, "%s missing metatable", T::className);
        int mt = lua_gettop(L);
        subtable(L, mt, "userdata", "v");
        User_Data *ud =
            static_cast<User_Data*>(pushuserdata(L, obj, sizeof(User_Data)));
        if (ud)
        {
            ud->pT = obj;  // store pointer to object in userdata
            lua_pushvalue(L, mt);
            lua_setmetatable(L, -2);
            if (gc == false)
            {
                lua_checkstack(L, 3);
                subtable(L, mt, "do not trash", "k");
                lua_pushvalue(L, -2);
                lua_pushboolean(L, 1);
                lua_settable(L, -3);
                lua_pop(L, 1);
            }
        }
        lua_replace(L, mt);
        lua_settop(L, mt);
        return mt;  // index of userdata containing pointer to T object
    }

    // get userdata from Lua stack and return pointer to T object
    static T *check(lua_State *L, int narg)
    {
        User_Data *ud =
            static_cast<User_Data*>(luaL_checkudata(L, narg, T::className));
        if (!ud) luaL_typerror(L, narg, T::className);
        return ud->pT;  // pointer to T object
    }

private:
    mm_Lua_Object();  // hide default constructor

    // member function dispatcher
    static int thunk(lua_State *L)
    {
        // stack has userdata, followed by method args
        T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
        lua_remove(L, 1);  // remove self so member function args start at index 1
        // get member function from upvalue
        Registration_Record *l = static_cast<Registration_Record*>(lua_touserdata(L, lua_upvalueindex(1)));
        return (obj->*(l->mfunc))(L);  // call member function
    }

    // garbage collection metamethod
    static int gc_T(lua_State *L)
    {
        if (luaL_getmetafield(L, 1, "do not trash"))
        {
            lua_pushvalue(L, 1);  // dup userdata
            lua_gettable(L, -2);
            if (!lua_isnil(L, -1)) return 0;  // do not delete object
        }
        User_Data *ud = static_cast<User_Data*>(lua_touserdata(L, 1));
        T *obj = ud->pT;
        if (obj) delete obj;  // call destructor for T objects
        return 0;
    }

    static int tostring_T(lua_State *L)
    {
        char buff[32];
        User_Data *ud = static_cast<User_Data*>(lua_touserdata(L, 1));
        T *obj = ud->pT;
        sprintf(buff, "%p", obj);
        lua_pushfstring(L, "%s (%s)", T::className, buff);
        return 1;
    }

    static void set(lua_State *L, int table_index, const char *key)
    {
        lua_pushstring(L, key);
        lua_insert(L, -2);  // swap value and key
        lua_settable(L, table_index);
    }

    static void weaktable(lua_State *L, const char *mode)
    {
        lua_newtable(L);
        lua_pushvalue(L, -1);  // table is its own metatable
        lua_setmetatable(L, -2);
        lua_pushliteral(L, "__mode");
        lua_pushstring(L, mode);
        lua_settable(L, -3);   // metatable.__mode = mode
    }

    static void subtable(lua_State *L, int tindex, const char *name, const char *mode)
    {
        lua_pushstring(L, name);
        lua_gettable(L, tindex);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            lua_checkstack(L, 3);
            weaktable(L, mode);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_settable(L, tindex);
        }
    }

    static void *pushuserdata(lua_State *L, void *key, size_t sz)
    {
        void *ud = 0;
        lua_pushlightuserdata(L, key);
        lua_gettable(L, -2);     // lookup[key]
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);         // drop nil
            lua_checkstack(L, 3);
            ud = lua_newuserdata(L, sz);  // create new userdata
            lua_pushlightuserdata(L, key);
            lua_pushvalue(L, -2);  // dup userdata
            lua_settable(L, -4);   // lookup[key] = userdata
        }
        return ud;
    }
};

#endif // MM_SCRIPTABLE_OBJECTS_H


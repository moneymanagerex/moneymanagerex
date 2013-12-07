/*
 * Copyright (c) 2010-2013 Alexander Ames
 * Alexander.Ames@gmail.com
 * See Copyright Notice at the end of this file
 */

// API Summary:
//
// This is a set of utility functions that add to the functionalit of
// LuaWrapper. Over time I found myself repeating a few patterns, such as
// writing many trvial getter and setter functions, and wanting pass ownership
// of one object to another and have the Lua script properly handle that case.
//
// This file contains the additional functions that I've added but that do
// not really belong in the core API.

#ifndef LUAWRAPPERUTILS_HPP_
#define LUAWRAPPERUTILS_HPP_

#include "luawrapper.hpp"

#ifndef LUAW_NO_CXX11
#include <type_traits>
#endif

#ifndef LUAW_STD
#define LUAW_STD std
#endif

////////////////////////////////////////////////////////////////////////////////
//
// A set of templated luaL_check and lua_push functions for use in the getters
// and setters below
//
// It is often useful to override luaU_check, luaU_to and/or luaU_push to
// operate on your own simple types rather than register your type with
// LuaWrapper, especially with small objects.
//

template<typename U, typename = void>
struct luaU_Impl
{
    static U    luaU_check(lua_State* L, int      index);
    static U    luaU_to   (lua_State* L, int      index);
    static void luaU_push (lua_State* L, const U& value);
    static void luaU_push (lua_State* L,       U& value);
};

template<typename U> U    luaU_check(lua_State* L, int      index) { return luaU_Impl<U>::luaU_check(L, index); }
template<typename U> U    luaU_to   (lua_State* L, int      index) { return luaU_Impl<U>::luaU_to   (L, index); }
template<typename U> void luaU_push (lua_State* L, const U& value) {        luaU_Impl<U>::luaU_push (L, value); }
template<typename U> void luaU_push (lua_State* L,       U& value) {        luaU_Impl<U>::luaU_push (L, value); }

////////////////////////////////////////////////////////////////////////////////
//
// This is slightly different than the previous three functions in that you
// shouldn't need to write your own version of it, since it uses luaU_check
// automatically.
//
template<typename U>
U luaU_opt(lua_State* L, int index, const U& fallback = U())
{
    if (lua_isnil(L, index))
        return fallback;
    else
        return luaU_Impl<U>::luaU_check(L, index);
}

template<>
struct luaU_Impl<bool>
{
    static bool luaU_check(lua_State* L, int         index) { return lua_toboolean  (L, index) != 0; }
    static bool luaU_to   (lua_State* L, int         index) { return lua_toboolean  (L, index) != 0; }
    static void luaU_push (lua_State* L, const bool& value) {        lua_pushboolean(L, value); }
};

template<>
struct luaU_Impl<const char*>
{
    static const char* luaU_check(lua_State* L, int                index) { return luaL_checkstring(L, index); }
    static const char* luaU_to   (lua_State* L, int                index) { return lua_tostring    (L, index); }
    static void        luaU_push (lua_State* L, const char* const& value) {        lua_pushstring  (L, value); }
};

template<>
struct luaU_Impl<unsigned int>
{
    static unsigned int luaU_check(lua_State* L, int                 index) { return static_cast<unsigned int>(luaL_checkinteger(L, index)); }
    static unsigned int luaU_to   (lua_State* L, int                 index) { return static_cast<unsigned int>(lua_tointeger    (L, index)); }
    static void         luaU_push (lua_State* L, const unsigned int& value) {                                  lua_pushinteger  (L, value); }
};

template<>
struct luaU_Impl<int>
{
    static int  luaU_check(lua_State* L, int        index) { return static_cast<int>(luaL_checkinteger(L, index)); }
    static int  luaU_to   (lua_State* L, int        index) { return static_cast<int>(lua_tointeger    (L, index)); }
    static void luaU_push (lua_State* L, const int& value) {                         lua_pushinteger  (L, value); }
};

template<>
struct luaU_Impl<unsigned char>
{
    static unsigned char luaU_check(lua_State* L, int                  index) { return static_cast<unsigned char>(luaL_checkinteger(L, index)); }
    static unsigned char luaU_to   (lua_State* L, int                  index) { return static_cast<unsigned char>(lua_tointeger    (L, index)); }
    static void          luaU_push (lua_State* L, const unsigned char& value) {                                   lua_pushinteger  (L, value); }
};

template<>
struct luaU_Impl<char>
{
    static char luaU_check(lua_State* L, int         index) { return static_cast<char>(luaL_checkinteger(L, index)); }
    static char luaU_to   (lua_State* L, int         index) { return static_cast<char>(lua_tointeger    (L, index)); }
    static void luaU_push (lua_State* L, const char& value) {                          lua_pushinteger  (L, value); }
};

template<>
struct luaU_Impl<float>
{
    static float luaU_check(lua_State* L, int          index) { return static_cast<float>(luaL_checknumber(L, index)); }
    static float luaU_to   (lua_State* L, int          index) { return static_cast<float>(lua_tonumber    (L, index)); }
    static void  luaU_push (lua_State* L, const float& value) {                           lua_pushnumber  (L, value); }
};

template<>
struct luaU_Impl<double>
{
    static double luaU_check(lua_State* L, int           index) { return static_cast<double>(luaL_checknumber(L, index)); }
    static double luaU_to   (lua_State* L, int           index) { return static_cast<double>(lua_tonumber    (L, index)); }
    static void   luaU_push (lua_State* L, const double& value) {                            lua_pushnumber  (L, value); }
};

#ifndef LUAW_NO_CXX11
template<typename T>
struct luaU_Impl<T, typename LUAW_STD::enable_if<LUAW_STD::is_enum<T>::value>::type>
{
    static T    luaU_check(lua_State* L, int      index) { return static_cast<T>(luaL_checkinteger  (L, index)); }
    static T    luaU_to   (lua_State* L, int      index) { return static_cast<T>(lua_tointeger      (L, index)); }
    static void luaU_push (lua_State* L, const T& value) {        lua_pushnumber(L, static_cast<int>(value   )); }
};

template<typename T>
struct luaU_Impl<T*, typename LUAW_STD::enable_if<LUAW_STD::is_class<T>::value>::type>
{
    static T*   luaU_check( lua_State* L, int index) { return luaW_check<T>(L, index); }
    static T*   luaU_to   ( lua_State* L, int index) { return luaW_to   <T>(L, index); }
    static void luaU_push ( lua_State* L, T*& value) {        luaW_push <T>(L, value); }
    static void luaU_push ( lua_State* L, T*  value) {        luaW_push <T>(L, value); }
};
#endif

///////////////////////////////////////////////////////////////////////////////
//
// These are just some functions I've always felt should exist
//

template <typename U>
inline U luaU_getfield(lua_State* L, int index, const char* field)
{
#ifndef LUAW_NO_CXX11
    static_assert(!std::is_same<U, const char*>::value, 
        "luaU_getfield is not safe to use on const char*'s. (The string will be popped from the stack.)");
#endif
    lua_getfield(L, index, field);
    U val = luaU_to<U>(L, -1);
    lua_pop(L, 1);
    return val;
}

template <typename U>
inline U luaU_checkfield(lua_State* L, int index, const char* field)
{
#ifndef LUAW_NO_CXX11
    static_assert(!std::is_same<U, const char*>::value, 
        "luaU_checkfield is not safe to use on const char*'s. (The string will be popped from the stack.)");
#endif
    lua_getfield(L, index, field);
    U val = luaU_check<U>(L, -1);
    lua_pop(L, 1);
    return val;
}

template <typename U>
inline void luaU_setfield(lua_State* L, int index, const char* field, U val)
{
    luaU_push<U>(L, val);
    lua_setfield(L, luaW_correctindex(L, index, 1), field);
}

///////////////////////////////////////////////////////////////////////////////
//
// A set of trivial getter and setter templates. These templates are designed
// to call trivial getters or setters.
//
// There are four forms:
//  1. Getting or setting a public member variable that is of a primitive type
//  2. Getting or setting a public member variable that is a pointer to an
//     object
//  3. Getting or setting a private member variable that is of a primitive type
//     through a getter or setter
//  3. Getting or setting a private member variable that is is a pointer to an
//     object through a getter or setter
//
// The interface to all of them is the same however. In addition to plain
// getter and setter functions, there is a getset which does both - if an
// argument is supplied it attempts to set the value, and in either case it
// returns the value. In your lua table declaration in C++ rather than write
// individiual wrappers for each getter and setter you may do the following:
//
// static luaL_reg Foo_metatable[] =
// {
//     { "GetBar", luaU_get<Foo, bool, &Widget::GetBar> },
//     { "SetBar", luaU_set<Foo, bool, &Widget::SetBar> },
//     { "Bar", luaU_getset<Foo, bool, &Widget::GetBar, &Widget::SetBar> },
//     { NULL, NULL }
// }
//
// Getters and setters must have one of the following signatures:
//    void T::Setter(U value);
//    void T::Setter(U* value);
//    void T::Setter(const U& value);
//    U Getter() const;
//    U* Getter() const;
//
// In this example, the variable Foo::bar is private and so it is accessed
// through getter and setter functions. If Foo::bar were public, it could be
// accessed directly, like so:
//
// static luaL_reg Foo_metatable[] =
// {
//     { "GetBar", luaU_get<Foo, bool, &Widget::bar> },
//     { "SetBar", luaU_set<Foo, bool, &Widget::bar> },
//     { "Bar", luaU_getset<Foo, bool, &Widget::bar> },
// }
//
// In a Lua script, you can now use foo:GetBar(), foo:SetBar() and foo:Bar()
//

template <typename T, typename U, U T::*Member>
int luaU_get(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    luaU_push<U>(L, obj->*Member);
    return 1;
}

template <typename T, typename U, U* T::*Member>
int luaU_get(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    luaW_push<U>(L, obj->*Member);
    return 1;
}

template <typename T, typename U, U (T::*Getter)() const>
int luaU_get(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    luaU_push<U>(L, (obj->*Getter)());
    return 1;
}

template <typename T, typename U, const U& (T::*Getter)() const>
int luaU_get(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    luaU_push<U>(L, (obj->*Getter)());
    return 1;
}

template <typename T, typename U, U* (T::*Getter)() const>
int luaU_get(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    luaW_push<U>(L, (obj->*Getter)());
    return 1;
}

template <typename T, typename U, U T::*Member>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
        obj->*Member = luaU_check<U>(L, 2);
    return 0;
}

template <typename T, typename U, U* T::*Member>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
    {
        U* member = luaW_opt<U>(L, 2);
        obj->*Member = member;
    }
    return 0;
}

template <typename T, typename U, const U* T::*Member>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
    {
        U* member = luaW_opt<U>(L, 2);
        obj->*Member = member;
    }
    return 0;
}

template <typename T, typename U, const U* T::*Member>
int luaU_setandrelease(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
    {
        U* member = luaW_opt<U>(L, 2);
        obj->*Member = member;
        if (member)
            luaW_release<U>(L, member);
    }
    return 0;
}

template <typename T, typename U, void (T::*Setter)(U)>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
        (obj->*Setter)(luaU_check<U>(L, 2));
    return 0;
}

template <typename T, typename U, void (T::*Setter)(const U&)>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
        (obj->*Setter)(luaU_check<U>(L, 2));
    return 0;
}

template <typename T, typename U, void (T::*Setter)(U*)>
int luaU_set(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
    {
        U* member = luaW_opt<U>(L, 2);
        (obj->*Setter)(member);
    }
    return 0;
}

template <typename T, typename U, void (T::*Setter)(U*)>
int luaU_setandrelease(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj)
    {
        U* member = luaW_opt<U>(L, 2);
        (obj->*Setter)(member);
        if (member)
            luaW_release<U>(L, member);
    }
    return 0;
}

template <typename T, typename U, U T::*Member>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        obj->*Member = luaU_check<U>(L, 2);
        return 0;
    }
    else
    {
        luaU_push<U>(L, obj->*Member);
        return 1;
    }
}

template <typename T, typename U, U* T::*Member>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        U* member = luaW_opt<U>(L, 2);
        obj->*Member = member;
        return 0;
    }
    else
    {
        luaW_push<U>(L, obj->*Member);
        return 1;
    }
}

template <typename T, typename U, U* T::*Member>
int luaU_getsetandrelease(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        U* member = luaW_opt<U>(L, 2);
        obj->*Member = member;
        if (member)
            luaW_release<U>(L, member);
        return 0;
    }
    else
    {
        luaW_push<U>(L, obj->*Member);
        return 1;
    }
}

template <typename T, typename U, U (T::*Getter)() const, void (T::*Setter)(U)>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        (obj->*Setter)(luaU_check<U>(L, 2));
        return 0;
    }
    else
    {
        luaU_push<U>(L, (obj->*Getter)());
        return 1;
    }
}

template <typename T, typename U, U (T::*Getter)() const, void (T::*Setter)(const U&)>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        (obj->*Setter)(luaU_check<U>(L, 2));
        return 0;
    }
    else
    {
        luaU_push<U>(L, (obj->*Getter)());
        return 1;
    }
}

template <typename T, typename U, const U& (T::*Getter)() const, void (T::*Setter)(const U&)>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        (obj->*Setter)(luaU_check<U>(L, 2));
        return 0;
    }
    else
    {
        luaU_push<U>(L, (obj->*Getter)());
        return 1;
    }
}

template <typename T, typename U, U* (T::*Getter)() const, void (T::*Setter)(U*)>
int luaU_getset(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        U* member = luaW_opt<U>(L, 2);
        (obj->*Setter)(member);
        return 0;
    }
    else
    {
        luaW_push<U>(L, (obj->*Getter)());
        return 1;
    }
}

template <typename T, typename U, U* (T::*Getter)() const, void (T::*Setter)(U*)>
int luaU_getsetandrelease(lua_State* L)
{
    T* obj = luaW_check<T>(L, 1);
    if (obj && lua_gettop(L) >= 2)
    {
        U* member = luaW_opt<U>(L, 2);
        (obj->*Setter)(member);
        if (member)
            luaW_release<U>(L, member);
        return 0;
    }
    else
    {
        luaW_push<U>(L, (obj->*Getter)());
        return 1;
    }
}

#if !defined(_WIN32) && !defined(LUAW_NO_CXX11)

///////////////////////////////////////////////////////////////////////////////
//
// luaU_func is a special macro that expands into a simple function wrapper.
// Unlike the getter setters above, you merely need to name the function you
// would like to wrap.
//
// For example,
//
// struct Foo
// {
//     int DoSomething(int, const char*);
// };
//
// static luaL_reg Foo_metatable[] =
// {
//     { "DoSomething", luaU_func(&Foo::DoSomething) },
//     { NULL, NULL }
// }
//
// This macro will expand based on the function signature of Foo::DoSomething
// In this example, it would expand into the following wrapper:
//
//     luaU_push(luaW_check<T>(L, 1)->DoSomething(luaU_check<int>(L, 2), luaU_check<const char*>(L, 3)));
//     return 1;
//
// In this example there is only one member function called DoSomething. In some 
// cases there may be multiple overrides for a function. For those cases, an 
// additional macro has been provided, luaU_funcsig, which takes the entire
// function signature. The arguments to the macro reflect the order they would
// appear in the function signature: return type, type name, function name, and
// finally a list of all the argument types. For example:
//
// struct Foo
// {
//     int DoSomething (const char*);
//     int DoSomething (const char*, int);
// };
//
// const struct luaL_Reg Foo_metatable[] =
// {
//     {"DoSomething1", luaU_funcsig(int, Foo, DoSomething, const char*)) },
//     {"DoSomething1", luaU_funcsig(int, Foo, DoSomething, const char*, int)) },
//     { NULL, NULL }
// };
//
// These macros and it's underlying templates are somewhat experimental and some
// refinements are probably needed.  There are cases where it does not
// currently work and I expect some changes can be made to refine its behavior.
//

#define luaU_func(memberfunc) &luaU_FuncWrapper<decltype(memberfunc),memberfunc>::call
#define luaU_funcsig(returntype, type, funcname, ...) luaU_func(static_cast<returntype (type::*)(__VA_ARGS__)>(&type::funcname))

template<int... ints> struct luaU_IntPack { };
template<int start, int count, int... tail> struct luaU_MakeIntRangeType { typedef typename luaU_MakeIntRangeType<start, count-1, start+count-1, tail...>::type type; };
template<int start, int... tail> struct luaU_MakeIntRangeType<start, 0, tail...> { typedef luaU_IntPack<tail...> type; };
template<int start, int count> inline typename luaU_MakeIntRangeType<start, count>::type luaU_makeIntRange() { return typename luaU_MakeIntRangeType<start, count>::type(); }
template<class MemFunPtrType, MemFunPtrType MemberFunc> struct luaU_FuncWrapper;

template<class T, class ReturnType, class... Args, ReturnType(T::*MemberFunc)(Args...)>
struct luaU_FuncWrapper<ReturnType (T::*)(Args...), MemberFunc>
{
public:
    static int call(lua_State* L)
    {
        return callImpl(L, luaU_makeIntRange<2,sizeof...(Args)>());
    }

private:
    template<int... indices>
    static int callImpl(lua_State* L, luaU_IntPack<indices...>)
    {
        luaU_push<ReturnType>(L, (luaW_check<T>(L, 1)->*MemberFunc)(luaU_check<Args>(L, indices)...));
        return 1;
    }
};

#endif

///////////////////////////////////////////////////////////////////////////////
//
// Calls the copy constructor for an object of type T.
// Arguments may be passed in, in case they're needed for the postconstructor
//
// e.g.
//
// C++:
// luaL_reg Foo_Metatable[] =
// {
//     { "clone", luaU_clone<Foo> },
//     { NULL, NULL }
// };
//
// Lua:
// foo = Foo.new()
// foo2 = foo:clone()
//
template <typename T>
int luaU_clone(lua_State* L)
{
    // obj ...
    T* obj = new T(*luaW_check<T>(L, 1));
    lua_remove(L, 1); // ...
    int numargs = lua_gettop(L);
    luaW_push<T>(L, obj); // ... clone
    luaW_hold<T>(L, obj);
    luaW_postconstructor<T>(L, numargs);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// luaU_build is intended to be used to initialize many values by passing in a
// table. They keys of the table are used as function names, and values are
// used as arguments to the function. This is intended to be used on functions
// that are simple setters.
//
// For example, if luaU_build is set as the post constructor, you can
// initialize an object as so:
//
// f = Foo.new
// {
//     X = 10;
//     Y = 20;
// }
//
// After the object is constructed, luaU_build will do the equivalent of
// calling f:X(10) and f:Y(20).
//
template <typename T>
int luaU_build(lua_State* L)
{
    // obj {}
    lua_insert(L, -2); // {} obj
    if (lua_type(L, 1) == LUA_TTABLE)
    {
        for (lua_pushnil(L); lua_next(L, 1); lua_pop(L, 1))
        {
            // {} obj k v
            lua_pushvalue(L, -2); // {} obj k v k
            lua_gettable(L, -4); // {} obj k v ud[k]
            lua_pushvalue(L, -4); // {} obj k v ud[k] ud
            lua_pushvalue(L, -3); // {} obj k v ud[k] ud v
            lua_call(L, 2, 0); // {} obj k v
        }
        // {} ud
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Takes the object of type T at the top of the stack and stores it in on a
// table with the name storagetable, on the table at the specified index.
//
// You may manually call luaW_hold and luaW_release to handle pointer
// ownership, but it is often easier to simply store a Lua userdata on a table
// that is owned by its parent. This ensures that your object will not be
// prematurely freed, and that it can only be destoryed after its parent.
//
// e.g.
//
// Foo* parent = luaW_check<Foo>(L, 1);
// Bar* child = luaW_check<Bar>(L, 2);
// if (parent && child)
// {
//     luaU_store<Bar>(L, 1, "Children");
//     parent->AddChild(child);
// }
//
template <typename T>
void luaU_store(lua_State* L, int index, const char* storagetable, const char* key = NULL)
{
    // ... store ... obj
    lua_getfield(L, index, storagetable); // ... store ... obj store.storagetable
    if (key)
        lua_pushstring(L, key); // ... store ... obj store.storagetable key
    else
        LuaWrapper<T>::identifier(L, luaW_to<T>(L, -2)); // ... store ... obj store.storagetable key
    lua_pushvalue(L, -3); // ... store ... obj store.storagetable key obj
    lua_settable(L, -3); // ... store ... obj store.storagetable
    lua_pop(L, 1); // ... store ... obj
}

/*
 * Copyright (c) 2010-2011 Alexander Ames
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#endif // LUAWRAPPERUTILS_HPP_

#include <lua.hpp>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>


int main()
{
	lua_State* lua;
	lua = luaL_newstate();
	luaL_openlibs(lua);

	luaL_dofile(lua, "using_array.lua");

	lua_close(lua);
}

/************* Using userdata **************/

#define setfunction(f, n) (lua_pushcfunction(lua, f), lua_setfield(lua, -2, n))

typedef struct {
	unsigned int size;
	double data[0];
} Array;

// __index
int array_get(lua_State* lua)
{
	Array* arr = (Array*)lua_touserdata(lua, 1);
	int i = luaL_checkinteger(lua, 2);
	luaL_argcheck(lua, arr != NULL, 2, "expected an array");
	luaL_argcheck(lua, 0 < i && i <= arr->size, 2, "index out of range");
	lua_pushnumber(lua, arr->data[i-1]);
	return 1;
}

// __newindex
int array_set(lua_State* lua)
{
	Array* arr = (Array*)lua_touserdata(lua, 1);
	int i = luaL_checkinteger(lua, 2);
	double v = luaL_checknumber(lua, 3);	
	luaL_argcheck(lua, arr != NULL, 2, "expected an array");
	luaL_argcheck(lua, 0 < i && i <= arr->size, 2, "index out of range");
	arr->data[i-1] = v;
	return 0;
}

// __len
int array_size(lua_State* lua)
{
	luaL_checktype(lua, 1, LUA_TUSERDATA);
	Array* arr = (Array*)lua_touserdata(lua, 1);
	lua_pushinteger(lua, arr->size);
	return 1;
}

Array* createArray(lua_State* lua, size_t size)
{
	Array* arr = (Array*)lua_newuserdata(lua, sizeof(Array) + size * sizeof(double));
	arr->size = size;
		lua_newtable(lua); // array metatable
		setfunction(array_get, "__index");
		setfunction(array_set, "__newindex");
		setfunction(array_size, "__len");
	lua_setmetatable(lua, -2);
	return arr;
}

int array_new(lua_State* lua)
{
	int size = luaL_checkinteger(lua, 1);
 	createArray(lua, size);
	return 1;
}

int array_make(lua_State* lua)
{
	int top = lua_gettop(lua)-1;
	Array* arr = createArray(lua, top);
	for (int i = 0; i < top; i++)
	{
		arr->data[i] = luaL_checknumber(lua, i + 2);
	}
	return 1;
}

void usingUserdata(lua_State* lua)
{
	lua_settop(lua, 0);

	lua_newtable(lua);
	setfunction(array_new, "new");
	setfunction(array_size, "size");
		lua_newtable(lua);
		setfunction(array_make, "__call");
	lua_setmetatable(lua, 1);

	lua_setglobal(lua, "array");

	lua_getglobal(lua, "useArray");
	if(lua_pcall(lua, 0, 0, 0)) {
		puts(lua_tostring(lua, -1));
	}
}

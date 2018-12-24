#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>


void stacktutorial(lua_State* lua);
void manipulatingLuaTables(lua_State* lua);
void accessingLuaEnv(lua_State* lua);
void runningLuaCode(lua_State* lua);
void callingLuaFunction(lua_State* lua);
void callingCFunction(lua_State* lua);
void registry(lua_State* lua);
void upvaluesClosures(lua_State* lua);
void usingUserdata(lua_State* lua);
void buidingVectorTable(lua_State* lua);


int main()
{
	// All lua states is kept in lua_State struct
	lua_State* lua;

	// create a lua_State
	lua = luaL_newstate();

	// lua does not keep its standard library with itself.
	// you have to open the library to use it.
	luaL_openlibs(lua);

	// stacktutorial(lua);
	runningLuaCode(lua);
	// accessingLuaEnv(lua);
	// callingLuaFunction(lua);
	// callingCFunction(lua);
	// manipulatingLuaTables(lua);
	registry(lua);
	upvaluesClosures(lua);

	// usingUserdata(lua);
	// buidingVectorTable(lua);

	// when the program quit you should close lua_State
	lua_close(lua);
}



/************* Lua Stack operations **************/

void stacktutorial(lua_State* lua)
{
	// Lua runtime has a stack where all Lua API operate
	// Data exchange between C and Lua is done in this stack
	// any lua value can be pushed in the stack:
	lua_pushnil(lua);         // a space in the stack is reserved for a nil value
	lua_pushboolean(lua, 1);  // boolean == int
	lua_pushnumber(lua, 5.9); // number == double
	lua_pushinteger(lua, 64); // integer == int

	char arr[] = "Lua C API";
	char* hello = "Hello, world.";
	lua_pushlstring(lua, arr, sizeof arr);
	lua_pushstring(lua, hello); // lua_pushstring(l, s) == lua_pushlstring(l, s, sizeof s)

	// Values stored in the stack can be retrieved by giving the index where the value is stored
	// to the lua_to* functions that will recapture the value and convert it to a specific C type
	// Index -1 means the stack's top. -2 means one before the top, -3, two before, -4, ...

	// lua_tostring returns to a copy of a lua string. Do not use it in 
	// your program make a copy althought it's already one.

	const char* hello2 = lua_tostring(lua, -1); // extracting from the top
	assert(hello != hello2);
	
	size_t size;
	const char* api = lua_tolstring(lua, -2, &size); // extracting from the top - 1
	assert(size == sizeof arr);

	lua_Integer i = lua_tointeger(lua, -3); // extracting from the top - 2
	lua_Number d = lua_tonumber(lua, -4); // extracting from the top - 3
	int b = lua_toboolean(lua, -5); // extracting from the top - 4


	// thinking of extracting a nil value isn't that helpful but either way
	// if the value is nil then any lua_to* function will return 0 or NULL
	assert(lua_tointeger(lua, -6) == 0);
	assert(lua_tonumber(lua, -6) == 0);
	assert(lua_tostring(lua, -6) == NULL);


	// We can check the value's type by using lua_is* functions ...
	assert(lua_isstring(lua, -1));
	assert(lua_isinteger(lua, -3));
	assert(lua_isnumber(lua, -4));
	assert(lua_isboolean(lua, -5));

	// ... or by getting the type with lua_type. Lua specifies some types:
	assert(lua_type(lua, -1) == LUA_TSTRING);
	assert(lua_type(lua, -4) == LUA_TNUMBER);
	assert(lua_type(lua, -5) == LUA_TBOOLEAN);
	assert(lua_type(lua, -6) == LUA_TNIL);

	// we alse have LUA_TTABLE, LUA_TTHREAD, LUA_TUSERDATA, and LUA_TFUNCTION - LUA_TINTEGER not defined -
	// and lua_isfunction, lua_istable, lua_isthread, lua_isuserdata

	// lua_isnumber does not check whether the value has that specific type,
	// but whether the value can be converted to that type; lua_isstring is similar: in
	// particular, any number satisfies lua_isstring.

	// It is OK to call any of the lua_to* functions even when the given element
	// does not have an appropriate type. Function lua_toboolean works for any
	// type; lua_tolstring returns NULL for non-string values. The numeric functions,
	// however, have no way to signal a wrong type, so they simply return zero. Usually
	// you would need to call lua_isnumber to check the type, but Lua 5.2 introduced
	// the following new functions:
	
	int isnumber, isinteger;
	double number = lua_tonumberx(lua, -4, &isnumber);
	int integer = lua_tointegerx(lua, -3, &isinteger);
	assert(isnumber && isinteger);

	// The last out parameter returns a boolean that indicates whether the Lua value
	// is a number. (You can call those functions with NULL as the last parameter, if you
	// do not need that value. The old lua_to* functions are now implemented as
	// macros using this facility.)


	// lua_gettop get the number of elements in the stack, which is the index of the top element
	int top = lua_gettop(lua);

	// Function lua_settop sets the top (that is, the number of elements in the stack) to a 
	// specific value. If the previous top was higher than the new one, the function discards
	// the extra top values. Otherwise, it pushes nils on the stack to get the given size.

	lua_settop(lua, -1); // no effect on the stack
	lua_settop(lua, lua_gettop(lua) + 2); // expanding 2 more nil values.
	lua_settop(lua, lua_gettop(lua) - 2); // ignore the two last stack's elements
	lua_settop(lua, -3); // ignoring the current two last elements

	// #define lua_pop(L,n) lua_settop(L, -(n) - 1)
	lua_pop(lua, 1); // ignoring 1 element

	lua_settop(lua, 0); // emptying the stack
	assert(lua_gettop(lua) == 0);

	// pushing two values to the stack
	lua_pushinteger(lua, 4);
	lua_pushstring(lua, "Coding in Lua");
	
	// lua_pushvalue pushes on the stack a copy of the element at the given index
	lua_pushvalue(lua, -2);
	assert(lua_tointeger(lua, -1) == 4);
	assert(lua_gettop(lua) == 3);
	
	// lua_remove removes the element at the given index, shifting down
	// all elements on top of this position to fill in the gap
	lua_remove(lua, -1);
	assert(strcmp(lua_tostring(lua, -1), "Coding in Lua") == 0);

	// lua_insert moves the top element into the given position, shifting up
	// all elements on top of this position to open space
	lua_insert(lua, -2);
	assert(lua_tonumber(lua, -1) == 4);

	// lua_copy copies the value at one index to another, leaving the original untouched.
	lua_copy(lua, -1, -2); // copy from -1 to -2
	assert(lua_tointeger(lua, -2) == 4);


	// pushing more two values to the stack
	lua_pushinteger(lua, 200);
	lua_pushinteger(lua, 100);

	assert(lua_tointeger(lua, -1) == 100);
	assert(lua_tointeger(lua, -2) == 200);

	// lua_replace pops a value from the top and sets it as the
	// value of the given index, without moving anything
	lua_replace(lua, -2);

	assert(lua_tointeger(lua, -1) == 100);
	assert(lua_tointeger(lua, -2) == 4);

	lua_settop(lua, 0);
}


/************* Running Lua Code **************/

void runningLuaCode(lua_State* lua)
{
	// To load and run a lua code from a file we have to load and compile the file,
	// push the chunk (a function) on the stack and run it. luaL_dofile does all 
	// those operations and return a non-zero value if there was any error.

	if (luaL_dofile(lua, "demo.lua")) {
		puts(lua_tostring(lua, -1));
		return;
	}

	// luaL_dofile is defined as a macro ( luaL_loadfile(...) || luaL_pcall(...) )

	// luaL_loadfile loads and compiles a lua code from a file
	// and pushes on the stack a function (which is the loaded chunk)
	luaL_loadfile(lua, "demo.lua");
	assert(lua_isfunction(lua, -1));

	// lua_pcall calls the function and then pops it from the stack.
	lua_pcall(lua, 0, 0, 0); // this function is discuss in "Calling Lua Functions" topic

	// luaL_loadfile loads and compiles a lua code from a string
	// and pushes on the stack a function (which is the loaded chunk)
	luaL_loadstring(lua, "write(_, firstname, secondname, me)");
	lua_pcall(lua, 0, 0, 0);

	// luaL_dostring is defined as a macro ( luaL_loadstring(...) || luaL_pcall(...) )
	luaL_dostring(lua, "var = 12");
}

/************* Accessing Lua Environment **************/

void accessingLuaEnv(lua_State* lua)
{
	// lua_getglobal pushes onto the stack a value from the global table with a given key
	lua_getglobal(lua, "firstname");
	printf("First name: %s\n", lua_tostring(lua, -1));
	
	lua_getglobal(lua, "secondname");
	printf("Second name: %s\n", lua_tostring(lua, -1));

	printf("Full name: %s %s\n", 
		lua_tostring(lua, -2), 
		lua_tostring(lua, -1)
	);
	lua_pop(lua, 2);

	// lua_setglobal pops a value from the top of the stack and sets it as the new value of global name.
	lua_pushnumber(lua, 5.4);
	lua_setglobal(lua, "num");

	// getting a function named "printValues" and then calling it.
	lua_getglobal(lua, "printValues");
	lua_pcall(lua, 0, 0, 0);
}

/************* Calling Lua Functions **************/

void callingLuaFunction(lua_State* lua)
{
	// pushing the function onto the stack
	lua_getglobal(lua, "write");

	// lua_pcall calls the function that is in the top. tables also can be used for this task if its metatable has __call setup.
	// the second argument is the number of arguments that are expected to be pass to the function. the third on is the amount
	// of value that are expected to be returned. the last is the index from the stack - not pseudo-index - of a function
	// for error message handling - zero means no function is used. the function is popped after being call
	lua_pcall(lua, 0, 0, 0); // by passing zero to all we are ignoring the return values and not passing the expected amount of arguments.

	// to pass arguments when calling functions, values should be pushed immediately after the function being pushed
	// and the number of arguments should be specified to lua_pcall. after the call those values are popped from the stack
	// to retrieve the return values, specify amount of returned values is all needed. after the call, those values are
	// pushed and the argments are popped.
	
	// here we are passing 2 arguments and catching 3 return values, although the function has 1 parameter and return one value.
	lua_getglobal(lua, "write");
	lua_pushstring(lua, "Hello World"); // the first argument
	lua_pushstring(lua, "this value won't be used by the function because it expect one argument"); // the second argument
	lua_pcall(lua, 2, 3, 0);

	// if the third argument exceed the amount of returned values, the remaiders are nil.
	printf("returned: %s\n", lua_tostring(lua, -3)); // the first return value
	assert(lua_isnil(lua, -2)); // the second return value
	assert(lua_isnil(lua, -1)); // the last return value
	lua_pop(lua, 2);
}

/************* Calling C Functions from Lua **************/

int lualib_add(lua_State* lua);

void callingCFunction(lua_State* lua)
{
	// To be able to call a C function in lua, we need to push the function and then set it to the global table.
	// The function has to have a specific signature - as the lualib_add does.

	// lua_pushcfunction pushes onto the stack a C pointer to lua_CFunction. This is the function that will be call in lua.
	lua_pushcfunction(lua, lualib_add);
	lua_setglobal(lua, "add"); // now lualib_add can be call in lua by calling the global variable "add"

	// use lua_register to do this task. it's a macro to ( lua_pushcfunction(L, (func)), lua_setglobal(L, (key)) )
	lua_register(lua, "add", lualib_add);

	lua_getglobal(lua, "callc");
	lua_pushnumber(lua, 31);
	lua_pushnumber(lua, 2);
	lua_pcall(lua, 2, 0, 0);
}

int lualib_add(lua_State* lua)
{
	// the exchange of arguments and return values between Lua and C is done through lua_State
	// which contains a new stack with only the arguments passed being pushed in the order of the call.
	// you should pop the arguments and push any return values.
	// this function should return the number of values pushed for return values.	
	// because the function can receive any type of value, it's important to check if the type is the expected.
	double result;
	result = luaL_checknumber(lua, 1); // checks if the value of index 1 is number and returns it,
	result += luaL_checknumber(lua, 2);
	lua_pushnumber(lua, result);
	return 1;
}

/************* Manipulating Lua Tables **************/

void manipulatingLuaTables(lua_State* lua)
{
	// lua_newtable creates a table and pushes it onto the stack.
	lua_newtable(lua);

	// to set a value in a table, first push a value as a key, then another value and finally call lua_settable.
	// this function will set the value with the key in the table of the given index and, finally, pop the two values.
	lua_pushstring(lua, "PI");
	lua_pushnumber(lua, 3.14159265);
	lua_settable(lua, -3);

	// by using lua_setfield, the step of pushing the key can be skipped by passing the key as argument.
	lua_pushnumber(lua, 1.61803399);
	lua_setfield(lua, -2, "GOLDEN_RATIO");

	// setting the table in the global table.
	lua_setglobal(lua, "Math");

	// getting a function named "printValues" and then calling it.
	lua_getglobal(lua, "printValues");
	lua_pcall(lua, 0, 0, 0);

	// getting a table named "me" from the global table.
	lua_getglobal(lua, "me");

	// to get a value from a table, first push a value as the key then call lua_gettable, which will push onto the stack
	// the value from the table of the given index with the key previously pushed. The key is then popped from the stack.
	lua_pushstring(lua, "name");
	lua_gettable(lua, -2);
	printf("Name: %s\n", lua_tostring(lua, -1));
	lua_pop(lua, 1);

	// lua_getfield does all the previous task for getting a value from a table.
	lua_getfield(lua, -1, "age");
	printf("Age: %i\n", lua_tointeger(lua, -1));
	lua_pop(lua, 1);
}


/************* Using the Registry **************/

void registry(lua_State* lua)
{
	lua_settop(lua, 0);

	lua_pushinteger(lua, 200);
	lua_setfield(lua, LUA_REGISTRYINDEX, "mynum");
	assert(lua_gettop(lua) == 0);

	lua_getfield(lua, LUA_REGISTRYINDEX, "mynum");
	assert(lua_tointeger(lua, -1) == 200);
	assert(lua_gettop(lua) == 1);

	lua_settop(lua, 0);

	static char Key;
	lua_pushnumber(lua, 45);
	lua_rawsetp(lua, LUA_REGISTRYINDEX, &Key);

	lua_rawgetp(lua, LUA_REGISTRYINDEX, &Key);
	assert(lua_tointeger(lua, -1) == 45);

	lua_settop(lua, 0);

	lua_pushinteger(lua, 77);
	int index = luaL_ref(lua, LUA_REGISTRYINDEX);
	assert(lua_gettop(lua) == 0);

	lua_rawgeti(lua, LUA_REGISTRYINDEX, index);
	assert(lua_tointeger(lua, -1) == 77);
	assert(lua_gettop(lua) == 1);

	lua_settop(lua, 0);

	lua_pushinteger(lua, 99);
	lua_rawseti(lua, LUA_REGISTRYINDEX, index);
	assert(lua_gettop(lua) == 0);

	lua_rawgeti(lua, LUA_REGISTRYINDEX, index);
	assert(lua_tointeger(lua, -1) == 99);
	assert(lua_gettop(lua) == 1);

	lua_settop(lua, 0);

	luaL_unref(lua, LUA_REGISTRYINDEX, index);
}

/************* Upvalues and Closures **************/

int counter(lua_State* lua)
{
	int count = lua_tointeger(lua, lua_upvalueindex(1));
	lua_pushinteger(lua, ++count);
	lua_pushvalue(lua, -1);
	lua_replace(lua, lua_upvalueindex(1));
	return 1;
}

int newCounter (lua_State *L) {
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, counter, 1);
	return 1;
}

#define text(x) #x

void upvaluesClosures(lua_State* lua)
{
	lua_pushinteger(lua, 0);
	lua_pushcclosure(lua, counter, 1);
	lua_setglobal(lua, "count");

	lua_register(lua, "newcounter", newCounter);
	
	luaL_loadstring(lua, text(
		local counter = newcounter();
		local c = newcounter();
		print("c", c(), c(), c())
		print("counter", counter(), counter(), counter(), counter())
		print("count", count(), count(), count())
	));

	lua_pcall(lua, 0, 0, 0);
}

/************* Using userdata **************/

#define setfunction(f, n) (lua_pushcfunction(lua, f), lua_setfield(lua, -2, n))

typedef struct {
	unsigned int size;
	double data[0];
} Array;

int array_get(lua_State* lua)
{
	Array* arr = lua_touserdata(lua, 1);
	int i = luaL_checkinteger(lua, 2);
	luaL_argcheck(lua, arr != NULL, 2, "expected an array");
	luaL_argcheck(lua, 0 < i && i <= arr->size, 2, "index out of range");
	lua_pushnumber(lua, arr->data[i-1]);
	return 1;
}

int array_set(lua_State* lua)
{
	Array* arr = lua_touserdata(lua, 1);
	int i = luaL_checkinteger(lua, 2);
	double v = luaL_checknumber(lua, 3);	
	luaL_argcheck(lua, arr != NULL, 2, "expected an array");
	luaL_argcheck(lua, 0 < i && i <= arr->size, 2, "index out of range");
	arr->data[i-1] = v;
	return 0;
}

int array_size(lua_State* lua)
{
	luaL_checktype(lua, 1, LUA_TUSERDATA);
	Array* arr = lua_touserdata(lua, 1);
	lua_pushinteger(lua, arr->size);
	return 1;
}

Array* createArray(lua_State* lua, size_t size)
{
	Array* arr = lua_newuserdata(lua, sizeof(Array) + size * sizeof(double));
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

/************* Building Vector Tables **************/

int vector_new(lua_State* lua);
int vector_distance(lua_State* lua);


void buidingVectorTable(lua_State* lua)
{
	lua_settop(lua, 0);

	lua_newtable(lua);
	setfunction(vector_distance, "distance");
	setfunction(vector_new, "new");
		lua_newtable(lua);
		setfunction(vector_new, "__call");
	lua_setmetatable(lua, 1);

	lua_setglobal(lua, "Vector");

	lua_getglobal(lua, "useVector");
	if (lua_pcall(lua, 0, 0, 0)){
		puts(lua_tostring(lua, -1));
	}
}

int vector_new(lua_State* lua)
{
	if (lua_isnoneornil(lua, 2)) lua_pushnumber(lua, 0);
	if (lua_isnoneornil(lua, 3)) lua_pushnumber(lua, 0);

	lua_newtable(lua);
	lua_insert(lua, 1);

	lua_setfield(lua, 1, "y");
	lua_setfield(lua, 1, "x");

	lua_newtable(lua);
	lua_insert(lua, 2);
	lua_setfield(lua, 2, "__index");

	lua_setmetatable(lua, 1);
	return 1;
}

#define getfield(table, coord) (lua_getfield(lua, table, coord), lua_tonumber(lua, -1))

int vector_distance(lua_State* lua)
{
	luaL_checktype(lua, 1, LUA_TTABLE);
	luaL_checktype(lua, 2, LUA_TTABLE);

	double x = getfield(1, "x");
	double y = getfield(1, "y");
	x -= getfield(2, "x");
	y -= getfield(2, "y");

	lua_settop(lua, 0);
	lua_pushnumber(lua, sqrt(x*x + y*y));
	return 1;
}



struct vxWindow
{
	HWND hwnd;
	HDC hdc;
	HGLRC glcontext;
};

void vxWindow_open(vxWindow* window)
{
}

int lua_vxWindow_open(lua_State* lua)
{
	vxWindow* window = (vxWindow*) lua_touserdata(lua, -1);
	vxWindow_open(window);
	
}

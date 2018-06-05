#include "LuaModule.h"
#include "lua.h"
#include "luascript.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "SkyMockInterface.h"
#include "luatinker.h"
LuaModule::LuaModule()
{
}


LuaModule::~LuaModule()
{
}
int cpp_func(int arg1, int arg2)
{
	return arg1 + arg2;
}

extern SkyMockInterface g_mockInterface;
bool LuaModule::InitLua()
{
	// Lua �� �ʱ�ȭ �Ѵ�.
	lua_State* L = luaL_newstate();

	// Lua �⺻ �Լ����� �ε��Ѵ�.- print() ���
	luaopen_base(L);

	// LuaTinker �� �̿��ؼ� �Լ��� ����Ѵ�.
	luatinker::def(L, "cpp_func", cpp_func);

	// sample1.lua ������ �ε�/�����Ѵ�.
	luatinker::dofile(L, "sample1.lua");

	// sample1.lua �� �Լ��� ȣ���Ѵ�.
	int result = luatinker::call<int>(L, "lua_func", 3, 4);

	// lua_func(3,4) �� ����� ���
	//printf("lua_func(3,4) = %d\n", result);

	// ���α׷� ����
	lua_close(L);

	/*LuaScript* script = new LuaScript("Player.lua");
	float posX = script->get<float>("player.position.x");
	float posY = script->get<float>("player.position.y");
	std::string filename = script->get<std::string>("player.filename");
	int hp = script->get<int>("player.HP");

	//std::cout << "Position X = " << posX << ", Y = " << posY << std::endl;
	//std::cout << "Filename:" << filename << std::endl;
	//std::cout << "HP:" << hp << std::endl;

	// getting arrays
	std::vector<int> v = script->getIntVector("array");
	//std::cout << "Contents of array:";
	for (std::vector<int>::Iterator it = v.begin();
		it != v.end();
		it++) {
		g_mockInterface.g_printInterface.sky_printf("%d\n", *it);
	//	std::cout << *it << ",";
	}
	//std::cout << std::endl;

	// getting table keys:
	std::vector<std::string> keys = script->getTableKeys("player");
	//std::cout << "Keys of [player] table:";
	for (std::vector<std::string>::Iterator it = keys.begin();
		it != keys.end();
		it++) {
		g_mockInterface.g_printInterface.sky_printf("%s\n", *it);
		//std::cout << *it << ",";
	}
	//std::cout << std::endl;*/
	return true;
}

bool LuaModule::DoFile(char* fileName)
{

	return false;
}

bool LuaModule::CloseLua()
{
	
	return true;
}

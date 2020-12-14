/*
 * Copyright (C) 2020 rizlone
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.h"
#include "sampgdk.h"

#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include <algorithm>
#include <queue>
#include <string>

IndexMap indexMap;
MainMap mainMap;

logprintf_t logprintf;

std::string getString(AMX *amx, cell param, bool toLower)
{
	std::string name;
	char *string = NULL;
	amx_StrParam(amx, param, string);
	if (string)
	{
		name = string;
		if (toLower)
		{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		}
	}
	return name;
}


PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return sampgdk::Supports() | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	
	logprintf("\n\n************************************");
	logprintf("\tPVar plugin loaded");
	logprintf("\t   Version: %s ", PLUGIN_VERSION);
	logprintf("\t   By: rizlone");
	logprintf("************************************\n");
	return sampgdk::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	logprintf("\n\nPVar plugin v%s by rizlone unloaded \n", PLUGIN_VERSION);
	sampgdk::Unload();
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason) 
{
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		IndexMap::iterator k = indexMap.find(playerid);
		if (k != indexMap.end())
		{
			indexMap.erase(k);
			mainMap.erase(i);
		}
	}
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarSetInt(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "PVarSetInt");
	std::string name = getString(amx, params[2], true);
	int value = static_cast<int>(params[3]), playerid = static_cast<int>(params[1]), index = 0;
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(playerid);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[playerid] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[playerid][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetInt(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "PVarGetInt");
	std::string name = getString(amx, params[2], true);
	int playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(int))
			{
				int value = boost::get<int>(j->second.get<1>());
				return static_cast<cell>(value);
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarSetString(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "PVarSetString");
	std::string name = getString(amx, params[2], true), value = getString(amx, params[3], false);
	int playerid = static_cast<int>(params[1]), index = 0;
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(playerid);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[playerid] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[playerid][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetString(AMX *amx, cell *params)
{
	CHECK_PARAMS(4, "PVarGetString");
	std::string name = getString(amx, params[2], true);
	int size = static_cast<int>(params[4]), playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(std::string))
			{
				cell *dest = NULL;
				amx_GetAddr(amx, params[3], &dest);
				amx_SetString(dest, boost::get<std::string>(j->second.get<1>()).c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarSetFloat(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "PVarSetFloat");
	std::string name = getString(amx, params[2], true);
	float value = amx_ctof(params[3]);
	int playerid = static_cast<int>(params[1]), index = 0;
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(playerid);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[playerid] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[playerid][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetFloat(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "PVarGetFloat");
	std::string name = getString(amx, params[2], true);
	int playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(float))
			{
				float value = boost::get<float>(j->second.get<1>());
				return amx_ftoc(value);
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarDelete(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "PVarDelete");
	std::string name = getString(amx, params[2], true);
	int playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			IndexMap::iterator k = indexMap.find(playerid);
			if (k != indexMap.end())
			{
				k->second.get<1>().push(j->second.get<0>());
			}
			i->second.erase(j);
			if (i->second.empty())
			{
				indexMap.erase(k);
				mainMap.erase(i);
			}
			return 1;
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarsGetUpperIndex(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "PVarsGetUpperIndex");
	int playerid = static_cast<int>(params[1]), index = 0;
	IndexMap::iterator k = indexMap.find(playerid);
	if (k != indexMap.end())
	{
		if (k->second.get<1>().empty())
		{
			index = k->second.get<0>();
			return static_cast<cell>(index + 1);
		}
	}
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		for (DataMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->second.get<0>() > index)
			{
				index = j->second.get<0>();
			}
		}
		return static_cast<cell>(index + 1);
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarGetNameAtIndex(AMX *amx, cell *params)
{
	CHECK_PARAMS(4, "PVarGetNameAtIndex");
	int index = static_cast<int>(params[2]), size = static_cast<int>(params[4]), playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		for (DataMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->second.get<0>() == index)
			{
				cell *dest = NULL;
				amx_GetAddr(amx, params[3], &dest);
				amx_SetString(dest, j->first.c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarGetType(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "PVarGetType");
	std::string name = getString(amx, params[2], true);
	int playerid = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(int))
			{
				return static_cast<cell>(PVAR_TYPE_INT);
			}
			if (j->second.get<1>().type() == typeid(std::string))
			{
				return static_cast<cell>(PVAR_TYPE_STRING);
			}
			if (j->second.get<1>().type() == typeid(float))
			{
				return static_cast<cell>(PVAR_TYPE_FLOAT);
			}
		}
	}
	return static_cast<cell>(PVAR_TYPE_NONE);
}

AMX_NATIVE_INFO natives[] =
{
	{ "PVarSetInt", n_PVarSetInt },
	{ "PVarGetInt", n_PVarGetInt },
	{ "PVarSetString", n_PVarSetString },
	{ "PVarGetString", n_PVarGetString },
	{ "PVarSetFloat", n_PVarSetFloat },
	{ "PVarGetFloat", n_PVarGetFloat },
	{ "PVarDelete", n_PVarDelete },
	{ "PVarsGetUpperIndex", n_PVarsGetUpperIndex },
	{ "PVarGetNameAtIndex", n_PVarGetNameAtIndex },
	{ "PVarGetType", n_PVarGetType },
	
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	return AMX_ERR_NONE;
}

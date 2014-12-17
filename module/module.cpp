// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Bot Played Time Faker Module
//

#include "module.h"
#include "hook.h"
#include "bot.h"

void OnMetaAttach()
{
	if (!CreateSendtoHook())
	{
		SERVER_PRINT(UTIL_VarArgs("\n   [%s] Could not create \"sendto\" hook\n\n", MODULE_NAME));
	}
}

void OnMetaDetach(void)
{
	SendtoDetour->Destroy();
}

BOOL OnClientConnect(edict_t* pEntity, const char* pszName, const char *pszAddress, char szRejectReason[128])
{
	BotsList[ENTINDEX(pEntity)].clear();

	RETURN_META_VALUE(MRES_IGNORED, FALSE);
}

void OnClientDisconnect(edict_t* pEntity)
{
	BotsList[ENTINDEX(pEntity)].clear();

	RETURN_META(MRES_IGNORED);
}

void OnClientUserInfoChangedPost(edict_t* pEntity, char* infobuffer)
{
	Bot* player = &BotsList[ENTINDEX(pEntity)];

	if (!player->isInGame() && player->isBot(pEntity))
	{
		player->setData(INFOKEY_VALUE(infobuffer, "name"));
	}

	RETURN_META(MRES_IGNORED);
}

void OnSetClientKeyValue(int clientIndex, char* infobuffer, const char* key, const char* value)
{
	Bot* player = &BotsList[clientIndex];

	if (*value == '1' && !player->isInGame() && !strcmp(key, "*bot"))
	{
		player->setData(INFOKEY_VALUE(infobuffer, "name"));
	}

	RETURN_META(MRES_IGNORED);
}

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

#include "bot.h"
#include "utils.h"

CBot Players[MaxClients + 1];

CBot::CBot(void)
{
	clear();
}

CBot::~CBot(void)
{
	
}

void CBot::clear(void)
{
	m_isBot       = false;
	m_inGame      = false;
	m_connectTime = 0;
	m_sayTime     = 0;
	m_name[0]     = '\0';
}

bool CBot::isInGame(void)
{
	return m_inGame;
}

void CBot::setData(const char* name)
{
	memcpy(m_name, name, strlen(name) + 1);

	m_sayTime     = 60 * RANDOM_FLOAT(30, 160);
	m_connectTime = GetTimeInSeconds() - m_sayTime * RANDOM_FLOAT(0.2, 0.8);
	m_isBot       = true;
	m_inGame      = true;
}


bool CBot::isBot(edict_t* pEntity)
{
	if ((pEntity->v.flags & FL_FAKECLIENT) == FL_FAKECLIENT)
	{
		return true;
	}

	const char* auth = GETPLAYERAUTHID(pEntity);

	if (auth && strcmp(auth, "BOT") == 0)
	{
		return true;
	}

	return false;
}


CBot* CBot::getBot(const char* name)
{
	for (size_t i = 1; i <= MaxClients; ++i)
	{
		CBot* bot = &Players[i];

		if (bot->m_isBot && !strcmp(bot->m_name, name))
		{
			return bot;
		}
	}

	return NULL;
}

void CBot::replaceConnectionTime(const char* name, float* timeslot)
{
	CBot* bot = getBot(name);

	if (bot)
	{
		float currentTime = GetTimeInSeconds();

		// Check if stay time has been exceeded
		if (currentTime - bot->m_connectTime > bot->m_sayTime || currentTime - bot->m_connectTime <= 0)
		{
			// Use system wide timer for connection times
			// Bot will stay 30-160 minutes
			bot->m_sayTime = 60 * RANDOM_FLOAT(30, 160);

			// Bot has been already here for 20%-80% of the total stay time
			bot->m_connectTime	= currentTime - bot->m_sayTime * RANDOM_FLOAT(0.2, 0.8);
		}

		*timeslot = (float)(currentTime - bot->m_connectTime);
	}
}

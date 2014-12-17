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

#ifndef BOT_H
#define BOT_H

#include "amxxmodule.h"

const int MaxClients = 32;

class Bot
{
	public:

		Bot(void);
		~Bot(void);

		void clear(void);
		bool isInGame(void);
		void setData(const char* name);

		static Bot* getBot(const char* name);
		static bool isBot(edict_t* pEntity);
		static void replaceConnectionTime(const char* name, float* timeslot);

	private:

		bool	m_isBot;
		bool	m_inGame;
		float	m_connectTime;
		float	m_sayTime;
		char	m_name[32];
};

extern Bot BotsList[];

#endif // BOT_H
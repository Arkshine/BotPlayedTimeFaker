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

#ifndef HOOK_H
#define HOOK_H

#include "CDetour/detours.h"

extern CDetour* SendtoDetour;

bool CreateSendtoHook(void);
bool MessageGetString(const unsigned char* &msg, size_t &len, char* name, int nlen);

#endif // HOOK_H

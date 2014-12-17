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

#include "hook.h"
#include "bot.h"
#include <am-string.h>

#if defined(WIN32)
	#include <sys/types.h>
	#include <string.h>

	typedef int socklen_t;
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <unistd.h>

	#define PASCAL
#endif

CDetour* SendtoDetour;

using namespace ke;

DETOUR_DECLCONV_STATIC6(OnNewSendto, size_t, PASCAL, int, socket, const void*, message, size_t, length, int, flags, const struct sockaddr*, dest_addr, socklen_t, dest_len)
{
	const unsigned char* origMessage = (unsigned char*)message;

	size_t ret = 0;

	if (length > 5 && origMessage[0] == 0xff && origMessage[1] == 0xff && origMessage[2] == 0xff && origMessage[3] == 0xff)
	{
		// check if this is player reply packet (ÿÿÿÿD)
		if (origMessage[4] == 'D')
		{
			/*
			(byte)		Character 'D'
			(byte)		Player Count
			--
			(byte)		Player Number
			(string)	Player Name
			(long)		Player Score
			(float)		Player Time
			--
			*/

			AString newMessage((const char*)origMessage, length);

			size_t currentLength = length - 5;
			const unsigned char* messageWithoutHeader = origMessage + 5;

			int		playerCount;
			char	playerName[32];

			playerCount = *messageWithoutHeader++; // Get player count

			if (--currentLength == 0)
			{
				goto skip;
			}

			for (int i = 0; i < playerCount; ++i)  // Parse player slots
			{
				messageWithoutHeader++;            // Skip player number

				if (--currentLength == 0)
				{
					break;
				}

				memset(playerName, 0, sizeof(playerName));

				if (!MessageGetString(messageWithoutHeader, currentLength, playerName, sizeof(playerName)))	 // Detect bot by name
				{
					break;
				}

				if (currentLength <= 4)	 // Skip score
				{
					break;
				}

				messageWithoutHeader += 4;
				currentLength -= 4;

				if (currentLength < 4)  // Check th at there is enough bytes left 
				{
					break;
				}

				CBot::replaceConnectionTime(playerName, (float*)&newMessage.chars()[(size_t)messageWithoutHeader - (size_t)message]);

				messageWithoutHeader+=4;
				currentLength -= 4;

				if (currentLength == 0)
				{
					break;
				}
			}

			return DETOUR_STATIC_CALL(OnNewSendto)(socket, newMessage.chars(), length, flags, dest_addr, dest_len);
		}
	}

skip:

	return DETOUR_STATIC_CALL(OnNewSendto)(socket, message, length, flags, dest_addr, dest_len);
}

bool CreateSendtoHook(void)
{
	#if defined(WIN32)

		void* address = (void*)GetProcAddress(GetModuleHandle("wsock32.dll"), "sendto");

		if (!address)
		{
			address = (void*)GetProcAddress(GetModuleHandle("Ws2_32.dll"), "sendto"); // Don't know if really need.
		}

	#else

		// metamod-p parses elf structures, we find function easier & better way
		void* address = (void*)&sendto;

		while (*(unsigned short*)address == 0x25ff)
		{
			address = **(void***)((char*)address + 2);
		}

	#endif

	SendtoDetour = DETOUR_CREATE_STATIC_FIXED(OnNewSendto, address);

	if (SendtoDetour)
	{
		SendtoDetour->EnableDetour();
		return true;
	}

	return false;
}

bool MessageGetString(const unsigned char* &buffer, size_t &len, char* name, int nlen)
{
	int i = 0;

	while (*buffer != 0)
	{
		if (i < nlen)
		{
			name[i++]= *buffer;
		}

		++buffer;

		if (--len == 0)
		{
			return false;
		}
	}

	if (i < nlen)
	{
		name[i] = 0;
	}

	++buffer;

	if (--len == 0)
	{
		return false;
	}

	return true;
}


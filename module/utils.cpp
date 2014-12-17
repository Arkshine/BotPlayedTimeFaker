// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Bot Played Time Faker Module
//

#include "utils.h"

#if defined(WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOWINRES
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

float GetTimeInSeconds(void)
{
	#if defined(WIN32)
		LARGE_INTEGER count, freq;

		count.QuadPart = 0;
		freq.QuadPart = 0;

		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&count);

		return (float)count.QuadPart / (float)freq.QuadPart;
	#else
		struct timeval tv;

		gettimeofday(&tv, NULL);

		return (float)tv.tv_sec + ((float)tv.tv_usec) / 1000000.0;
	#endif
}
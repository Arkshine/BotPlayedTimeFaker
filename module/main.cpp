#include "amxxmodule.h"
#include "chooker.h"
#include "main.h"

CHooker		HookerClass;
CHooker*	Hooker = &HookerClass;

PlayerData Players[32];

Func_Sendto SendtoOriginal = NULL;
CFunc* SendtoHook = NULL;

void OnMetaAttach()
{
	bool result = hookSendto();

	printf("\n %s v%s - by %s.\n -\n", MODULE_NAME, MODULE_VERSION, MODULE_AUTHOR);
	printf(" %s.\n\n", result ? "Loaded with success" : "Failed to create hook");
}

BOOL OnClientConnect(edict_t* pEntity, const char* pszName, const char *pszAddress, char szRejectReason[128])
{
	PlayerData &pBot = Players[ENTINDEX(pEntity) - 1];
	memset(&pBot, 0, sizeof(PlayerData));

	RETURN_META_VALUE(MRES_IGNORED, FALSE);
}

void OnClientDisconnect(edict_t* pEntity)
{
	PlayerData &pBot = Players[ENTINDEX(pEntity) - 1];
	memset(&pBot, 0, sizeof(PlayerData));

	RETURN_META(MRES_IGNORED);
}

void OnClientUserInfoChangedPost(edict_t* pEntity, char* infobuffer)
{
	int index = ENTINDEX(pEntity) - 1;

	if (!Players[index].inGame && isBot(pEntity))
	{
		const char* name = INFOKEY_VALUE(infobuffer, "name");
		setBotDatas(index, name);
	}

	RETURN_META(MRES_IGNORED);
}

void OnSetClientKeyValue(int clientIndex, char* infobuffer, const char* key, const char* value)
{
	int index = clientIndex - 1;

	if (value[0] == '1' && !Players[index].inGame && !strcmp(key, "*bot"))
	{
		char* name = INFOKEY_VALUE(infobuffer, "name");
		setBotDatas(index, name);
	}

	RETURN_META(MRES_IGNORED);
}

void OnMetaDetach(void)
{
	SendtoHook->Restore();
}

size_t PASCAL OnNewSendto(int socket, const void* message, size_t length, int flags, const struct sockaddr* dest_addr, socklen_t dest_len)
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

			unsigned char* newMessage = (unsigned char*)malloc(length);
			memcpy(newMessage, message, length);

			size_t len = length - 5;
			const unsigned char* messageWithoutHeader = (const unsigned char*)message + 5;

			int		playerCount;
			char	playerName[32];
			size_t	offset;

			// Get player count
			playerCount = *messageWithoutHeader++;

			if (--len == 0)
				goto skip;

			// Parse player slots
			for (int i = 0; i < playerCount; i++)
			{
				// Skip player number
				messageWithoutHeader++;

				if (--len == 0)
					break;

				memset(playerName, 0, sizeof(playerName));

				// Detect bot by name
				if (!messageGetString(messageWithoutHeader, len, playerName, sizeof(playerName)))
					break;

				// Skip score
				if (len <= 4)
					break;

				messageWithoutHeader += 4;
				len -= 4;

				// Check th at there is enough bytes left
				if (len < 4)
					break;

				offset = (size_t)messageWithoutHeader - (size_t)message;

				botReplaceConnectionTime(playerName, (float*)&newMessage[offset]);

				messageWithoutHeader+=4;
				len-=4;

				if (len == 0)
					break;
			}

			if (SendtoHook->Restore())
			{
				ret = SendtoOriginal(socket, newMessage, length, flags, dest_addr, dest_len);
				SendtoHook->Patch();
			}

			return ret;
		}
	}

skip:

	if (SendtoHook->Restore())
	{
		ret = SendtoOriginal(socket, message, length, flags, dest_addr, dest_len);
		SendtoHook->Patch();
	}

	return ret;
}

void botReplaceConnectionTime(const char* name, float* timeslot)
{
	for (int i = 0; i < 32; i++)
	{
		PlayerData &bot = Players[i];

		if (bot.isBot && !strcmp(bot.name, name))
		{
			float currentTime = getSeconds();

			// Check if stay time has been exceeded
			if (currentTime - bot.connectTime > bot.sayTime || currentTime - bot.connectTime <= 0)
			{
				// Use system wide timer for connection times
				// Bot will stay 30-160 minutes
				bot.sayTime = 60 * RANDOM_FLOAT(30, 160);

				// Bot has been already here for 20%-80% of the total stay time
				bot.connectTime	= getSeconds() - bot.sayTime * RANDOM_FLOAT(0.2, 0.8);
			}

			*timeslot = (float)(currentTime - bot.connectTime);

			break;
		}
	}
}

bool hookSendto(void)
{
#ifdef WIN32

	SendtoOriginal = (Func_Sendto)GetProcAddress(GetModuleHandle("wsock32.dll"), "sendto");

#else

	// metamod-p parses elf structures, we find function easier & better way
	void * sym_ptr = ( void* )&sendto;

	while( *( unsigned short* )sym_ptr == 0x25ff )
	{
		sym_ptr = **( void*** )( ( char* )sym_ptr + 2 );
	}

	SendtoOriginal = ( Func_Sendto )sym_ptr;

#endif

	SendtoHook = Hooker->CreateHook(SendtoOriginal, (void*)OnNewSendto, TRUE);

	return SendtoHook ? true : false;
}

void setBotDatas(int index, const char* name)
{
	PlayerData &pBot = Players[index];

	memset(&pBot, 0, sizeof(PlayerData));

	memcpy(pBot.name, name, strlen(name) + 1);
	pBot.sayTime		= 60 * RANDOM_FLOAT(30, 160);
	pBot.connectTime	= getSeconds() - Players[index].sayTime * RANDOM_FLOAT(0.2, 0.8);
	pBot.isBot			= true;
	pBot.inGame			= true;
}

bool isBot(edict_t* pEntity)
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

bool messageGetString(const unsigned char* &msg, size_t &len, char* name, int nlen)
{
	int i = 0;

	while (*msg != 0)
	{
		if (i < nlen)
			name[i++]= *msg;

		msg++;

		if (--len == 0)
			return false;
	}

	if (i < nlen)
		name[i] = 0;

	msg++;

	if (--len == 0)
		return false;

	return true;
}

float getSeconds(void)
{
#ifdef _WIN32
	LARGE_INTEGER count, freq;

	count.QuadPart = 0;
	freq.QuadPart = 0;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);

	return (float)count.QuadPart / (float)freq.QuadPart;
#else
	struct timeval tv;

	gettimeofday( &tv, NULL );

	return ( float )tv.tv_sec + ((float) tv.tv_usec ) / 1000000.0;
#endif
}
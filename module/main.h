
#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>
	#include <sys/types.h>
	#include <string.h>

	typedef int socklen_t;
#else
	#include <sys/types.h>
	#include <sys/time.h> 
	#include <sys/socket.h>
	#include <unistd.h>

	#define PASCAL
#endif

#include <malloc.h>

typedef struct 
{
	bool	isBot;
	bool	inGame;
	float	connectTime;
	float	sayTime;
	char	name[ 32 ];

} PlayerData;

typedef size_t ( PASCAL *Func_Sendto )( int, const void*, size_t, int, const struct sockaddr*, socklen_t );

size_t PASCAL OnNewSendto( int socket, const void* message, size_t length, int flags, const struct sockaddr* dest_addr, socklen_t dest_len );

bool	hookSendto		( void );
void	setBotDatas		( int index, const char* name );
bool	messageGetString( const unsigned char* &msg, size_t &len, char* name, int nlen );
float	getSeconds		( void );
bool	isBot			( edict_t* pEntity );

void	botReplaceConnectionTime( const char* name, float* timeslot );


#endif // MAIN_H
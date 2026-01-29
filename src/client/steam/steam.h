#pragma once


// http://www.rohitab.com/discuss/topic/33780-simple-question-about-steamdll/



// #define STEAM_EXPORT extern "C" __declspec(dllexport)

#define STEAM_API extern "C" __declspec(dllexport)
#define STEAM_CALL __cdecl


typedef enum
{

} ESteamError;
typedef enum
{

} EDetailedPlatformErrorType;
typedef struct
{

} TSteamError;

#include "interfaces/client.h"

namespace steam
{
	STEAM_API int STEAM_CALL SteamIsAppSubscribed(unsigned int uAppId, int* pbIsAppSubscribed, TSteamError* pError);
	STEAM_API int STEAM_CALL SteamStartup(unsigned int uUsingMask, TSteamError* pError);
	STEAM_API int STEAM_CALL SteamCleanup(TSteamError* pError);
}
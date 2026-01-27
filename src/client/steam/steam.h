#pragma once

#include "nt.h"

#define STEAM_EXPORT extern "C" __declspec(dllexport)

#include "interfaces/apps.h"

namespace steam
{
	STEAM_EXPORT bool SteamAPI_Init();
	STEAM_EXPORT const char* SteamAPI_GetSteamInstallPath();
}
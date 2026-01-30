#include "pch.h"
#include "steam/steam.h"

#include "../components/steam_proxy.h"

namespace steam
{
#if 0
    extern "C"
    {
        // Calls order: SteamStartup > SteamIsAppSubscribed > SteamCleanup


        int SteamIsAppSubscribed(unsigned int uAppId, int* pbIsAppSubscribed, TSteamError* pError)
        {
            return 1;
        }
        int SteamStartup(unsigned int uUsingMask, TSteamError* pError)
        {











            return 1;
        }
        int SteamCleanup(TSteamError* pError)
        {
            return 1;
        }
    }
#endif
}
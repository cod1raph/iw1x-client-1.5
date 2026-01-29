#include "pch.h"
#include "steam/steam.h"

#include "../components/steam_proxy.h"

namespace steam
{

    
    extern "C"
    {




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
}
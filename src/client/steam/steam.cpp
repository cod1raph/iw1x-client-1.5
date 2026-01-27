#include "pch.h"
#include "steam/steam.h"

namespace steam
{
    const char* SteamAPI_GetSteamInstallPath()
    {
        static std::string install_path{};
        if (!install_path.empty())
        {
            return install_path.data();
        }
        
        HKEY reg_key;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
        {
            char path[MAX_PATH] = {0};
            DWORD length = sizeof(path);
            
            RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path), &length);
            RegCloseKey(reg_key);
            install_path = path;
        }
        return install_path.data();
    }
    
    extern "C"
    {
        bool SteamAPI_Init()
        {
            const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
            if (steam_path.empty())
            {
                return true;
            }
            
            ::utils::nt::library::load(steam_path / "gameoverlayrenderer.dll");
            ::utils::nt::library::load(steam_path / "steamclient.dll");
            
            return true;
        }
    }
}
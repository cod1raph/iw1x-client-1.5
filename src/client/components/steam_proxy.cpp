#include "pch.h"
#if 1
#include "steam_proxy.h"

#include "../steam/interface.h"
#include "../steam/steam.h"


#include "io.h"

namespace steam_proxy
{
    const auto app_id = 2620;
    
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
            char path[MAX_PATH] = { 0 };
            DWORD length = sizeof(path);

            RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path), &length);
            RegCloseKey(reg_key);
            install_path = path;
        }
        return install_path.data();
    }
    
    void load()
    {
        const std::filesystem::path steam_path = SteamAPI_GetSteamInstallPath();
        if (steam_path.empty()) return;

        SetEnvironmentVariableA("SteamAppId", ::utils::string::va("%lu", app_id));
        SetEnvironmentVariableA("SteamGameId", ::utils::string::va("%llu", app_id & 0xFFFFFF));
        utils::io::write_file("steam_appid.txt", utils::string::va("%lu", app_id), false);



        utils::nt::library::load(steam_path / "steam.dll");



        
    }
    
    class component final : public component_interface
    {
    public:
        void post_load() override
        {
            //load();
        }
    };
}

REGISTER_COMPONENT(steam_proxy::component)
#endif
#include "pch.h"
#if 1
#include "steam_proxy.h"

#include "steam/interface.h"
#include "steam/steam.h"

namespace steam_proxy
{
    class component final : public component_interface
    {
    public:
        void post_load() override
        {
#if 1
            const auto app_id = 2620;

            SetEnvironmentVariableA("SteamAppId", utils::string::va("%lu", app_id));
            SetEnvironmentVariableA("SteamGameId", utils::string::va("%llu", app_id & 0xFFFFFF));
            
            this->load_client();

            try
            {
                this->start_mod(app_id);
            }
            catch (std::exception& e)
            {
                /*std::stringstream ss;
                ss << "###### ERR: " << e.what() << std::endl;
                OutputDebugString(ss.str().c_str());*/
                MessageBox(NULL, e.what(), MOD_NAME, MB_ICONERROR | MB_SETFOREGROUND);
                //printf("Steam: %s\n", e.what());
            }
#endif
        }
        
        void pre_destroy() override
        {
            if (this->steam_client_module_)
            {
                if (this->steam_pipe_)
                {
                    if (this->global_user_)
                    {
                        this->steam_client_module_.invoke<void>("Steam_ReleaseUser", this->steam_pipe_,
                            this->global_user_);
                    }

                    this->steam_client_module_.invoke<bool>("Steam_BReleaseSteamPipe", this->steam_pipe_);
                }
            }
        }
        
        const utils::nt::library& get_overlay_module() const
        {
            return steam_overlay_module_;
        }

    private:
        utils::nt::library steam_client_module_{};
        utils::nt::library steam_overlay_module_{};

        steam::interface client_engine_{};
        steam::interface client_user_{};
        steam::interface client_utils_{};

        void* steam_pipe_ = nullptr;
        void* global_user_ = nullptr;

        void* load_client_engine() const
        {
            if (!this->steam_client_module_) return nullptr;
            
            for (auto i = 1; i > 0; ++i)
            {
                std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
                auto* const client_engine = this->steam_client_module_.invoke<void*>("CreateInterface", name.data(), nullptr);
                if (client_engine)
                    return client_engine;
            }
            return nullptr;
        }

        void load_client()
        {
            const std::filesystem::path steam_path = steam::SteamAPI_GetSteamInstallPath();
            if (steam_path.empty())
            {
                return;
            }

            utils::nt::library::load(steam_path / "tier0_s.dll");
            utils::nt::library::load(steam_path / "vstdlib_s.dll");
            this->steam_overlay_module_ = utils::nt::library::load(steam_path / "gameoverlayrenderer.dll");
            this->steam_client_module_ = utils::nt::library::load(steam_path / "steamclient.dll");
            if (!this->steam_client_module_) return;

            this->client_engine_ = load_client_engine();
            if (!this->client_engine_) return;

            this->steam_pipe_ = this->steam_client_module_.invoke<void*>("Steam_CreateSteamPipe");
            this->global_user_ = this->steam_client_module_.invoke<void*>("Steam_ConnectToGlobalUser", this->steam_pipe_);
            this->client_user_ = client_engine_.invoke<void*>(8, steam_pipe_, global_user_); // GetIClientUser
            this->client_utils_ = this->client_engine_.invoke<void*>(14, this->steam_pipe_); // GetIClientUtils
        }

        void start_mod(size_t app_id)
        {
            if (!this->client_utils_ || !this->client_user_) return;

            if (!this->client_user_.invoke<bool>("BIsSubscribedApp", app_id))
            {
                app_id = 480; // Spacewar
            }
            
            this->client_utils_.invoke<void>("SetAppIDForCurrentPipe", app_id, false);
        }
    };

    const utils::nt::library& get_overlay_module()
    {
        // TODO: Find a better way to do this
        return component_loader::get<component>()->get_overlay_module();
    }
}

REGISTER_COMPONENT(steam_proxy::component)
#endif
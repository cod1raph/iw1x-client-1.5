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
            this->load_client();
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
                auto* const client_engine = this->steam_client_module_
                    .invoke<void*>("CreateInterface", name.data(), nullptr);
                if (client_engine) return client_engine;
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
            this->global_user_ = this->steam_client_module_.invoke<void*>(
                "Steam_ConnectToGlobalUser", this->steam_pipe_);
            this->client_user_ = client_engine_.invoke<void*>(8, steam_pipe_, global_user_); // GetIClientUser
            this->client_utils_ = this->client_engine_.invoke<void*>(14, this->steam_pipe_); // GetIClientUtils
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
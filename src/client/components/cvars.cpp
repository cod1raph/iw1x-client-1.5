#include "pch.h"
#if 1
#include "cvars.h"

namespace cvars
{
    utils::hook::detour hook_Cvar_Set_f;
    utils::hook::detour hook_Cvar_Command;

    /*
    * TODO: For cleanliness, hook "Cvar_Set" instead of "Cvar_Set_f" and "Cvar_Command".
    * It seems it would require extracting "qboolean force" from eax register.
    */
    
    static void stub_Cvar_Set_f(void)
    {
        char* name = stock::Cmd_Argv(1);
        if (!_stricmp(name, view::cg_fovScale->name))
        {
            const float value = std::stof(stock::Cmd_Argv(2));
            if (value < view::cg_fovScale_min || value > view::cg_fovScale_max)
            {
                stock::Com_Printf("%s should be between %.0f and %.2f\n", name, view::cg_fovScale_min, view::cg_fovScale_max);
                return;
            }
        }
        hook_Cvar_Set_f.invoke();
    }

    static void stub_Cvar_Command(void)
    {
        if (stock::Cmd_Argc() == 2)
        {
            char* name = stock::Cmd_Argv(0);
            if (!_stricmp(name, view::cg_fovScale->name))
            {
                const float value = std::stof(stock::Cmd_Argv(1));
                if (value < view::cg_fovScale_min || value > view::cg_fovScale_max)
                {
                    stock::Com_Printf("%s should be between %.0f and %.2f\n", name, view::cg_fovScale_min, view::cg_fovScale_max);
                    return;
                }
            }
        }
        hook_Cvar_Command.invoke();
    }
    
    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            hook_Cvar_Set_f.create(0x0043c410, stub_Cvar_Set_f);
            hook_Cvar_Command.create(0x0043c210, stub_Cvar_Command);
        }
    };
}

REGISTER_COMPONENT(cvars::component)
#endif
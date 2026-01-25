#include "shared.h"

#include "hook.h"

#include "loader/component_loader.h"

namespace view
{
    constexpr float cg_fovScale_min = 1;
    constexpr float cg_fovScale_max = 1.25;
    
    extern stock::cvar_t* cg_fovScaleEnable;
    extern stock::cvar_t* cg_fovScale;
}
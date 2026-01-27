#pragma once

#include "nt.h"
#include "_string.h"

#include "loader/component_loader.h"

namespace steam_proxy
{
    const utils::nt::library& get_overlay_module();
}
namespace stock
{
    WEAK adjuster<cvar_t* (const char* name, const char* value, int flags)> Cvar_Get{ 0x0043b880 };
    WEAK adjuster<cvar_t* (const char* name, const char* value)> Cvar_Set{ 0x0043bbb0 };
    WEAK adjuster<LRESULT CALLBACK(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> MainWndProc{ 0x0046c160 };
    WEAK adjuster<void()> IN_DeactivateMouse{ 0x004669d0 };
    WEAK adjuster<void()> IN_ActivateMouse{ 0x00466a50 };
    WEAK adjuster<void(const char* cmd_name, xcommand_t function)> Cmd_AddCommand{ 0x0042a870 };
}
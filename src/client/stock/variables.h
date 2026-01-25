namespace stock
{
    constexpr auto MAX_STRING_CHARS = 1024;
    constexpr auto CVAR_ARCHIVE = 1;
    constexpr auto CVAR_USERINFO = 2;
    constexpr auto KEYCATCH_CONSOLE = 0x0001;
    constexpr auto KEYCATCH_UI = 0x0002;
    constexpr auto KEYCATCH_MESSAGE = 0x0004;
    constexpr auto KEYCATCH_CGAME = 0x0008;
    constexpr auto EF_MG42_ACTIVE = 0xc000;

    WEAK adjuster<int> cls_keyCatchers{ 0x015b8aa4 };
    WEAK adjuster<HWND> hWnd{ 0x01999d68 };
    WEAK adjuster<qboolean> mouseActive{ 0x0093b3a0 };
    WEAK adjuster<qboolean> mouseInitialized{ 0x0093b3a4 };
}
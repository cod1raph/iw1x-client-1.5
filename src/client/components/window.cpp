#include "pch.h"
#if 0
#include "window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace window
{
    char sys_cmdline[stock::MAX_STRING_CHARS];
    HHOOK hHook;

    utils::hook::detour hook_Com_Init;

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (GetForegroundWindow() == *stock::hWnd)
        {
            if (nCode == HC_ACTION)
            {
                auto pKey = (KBDLLHOOKSTRUCT*)lParam;

                auto altDown = (pKey->flags & LLKHF_ALTDOWN);
                auto escDown_sys = (pKey->vkCode == VK_ESCAPE && wParam == WM_SYSKEYDOWN);
                auto escUp_sys = (pKey->vkCode == VK_ESCAPE && wParam == WM_SYSKEYUP);
                
                if (altDown)
                {
                    if (escDown_sys)
                    {
                        if (!imgui::waitForMenuKeyRelease)
                        {
                            imgui::toggle_menu(false);
                            imgui::waitForMenuKeyRelease = true;
                        }
                        return 1; // Prevent Windows Alt+Esc behavior
                    }
                    else if (escUp_sys)
                    {
                        imgui::waitForMenuKeyRelease = false;
                    }
                }
                else if (imgui::waitForMenuKeyRelease)
                {
                    // Released Alt before releasing Esc
                    imgui::waitForMenuKeyRelease = false;
                }
            }
        }

        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }
    
    static LRESULT CALLBACK stub_MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (imgui::displayed && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;
        
        switch (uMsg)
        {
        case WM_CHAR:
            if (wParam == VK_ESCAPE && imgui::displayed)
                imgui::toggle_menu(false);
            break;
        case WM_KEYDOWN:
            if (imgui::displayed)
                return 0; // Prevent moving
            break;
        case WM_MOUSEWHEEL:
            if (imgui::displayed)
                return 0; // Prevent changing weapon
            break;
        case WM_MENUCHAR:
            return MNC_CLOSE << 16; // Prevent Alt+Enter beep sound
        }

        // See https://github.com/kartjom/CoDPlusPlus/blob/359539f889958b2cbd58884cbc5bb0e3e5a3c294/CoDPlusPlus/src/Utils/WinApiHelper.cpp#L210
        if (wParam == SC_KEYMENU && (lParam >> 16) <= 0)
        {
            /*
            When opening imgui, it can fail getting focus if Alt+Esc was not pressed in a (non-natural) particular way
            Returning here prevents this
            */
            if (imgui::displayed)
                return 0;

            /*
            When closing imgui, if game is windowed and console is open,
            console's text field might lose focus because of the Alt press, and the (non visible) system menu woult obtain it
            Returning here prevents this
            */
            if (*stock::cls_keyCatchers & stock::KEYCATCH_CONSOLE)
                return 0;
        }
        
        return stock::MainWndProc(hWnd, uMsg, wParam, lParam);
    }
    
    static void stub_Com_Init(char*)
    {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!hHook)
            throw std::runtime_error(utils::string::va("SetWindowsHookEx for LowLevelKeyboardProc failed"));
        
        hook_Com_Init.invoke(sys_cmdline);
    }

    static void Cmd_Minimize()
    {
        ShowWindow(*stock::hWnd, SW_MINIMIZE);
    }
    
    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            stock::Cmd_AddCommand("minimize", Cmd_Minimize);

            utils::hook::set(0x468db9 + 1, stub_MainWndProc);
            utils::hook::set(0x4EEBD1, 0x00); // Alt+Tab support, see https://github.com/xtnded/codextended-client/pull/1
            
            hook_Com_Init.create(0x00439a40, stub_Com_Init);
        }
    };
}

REGISTER_COMPONENT(window::component)
#endif
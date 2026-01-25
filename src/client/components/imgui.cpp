#include "pch.h"
#if 1
#include "imgui.h"

namespace imgui
{
#define BEGINTABITEM_SPACE(label) if (ImGui::BeginTabItem(label)) { ImGui::Dummy(ImVec2(0, 5));
#define ENDTABITEM_SPACED() ImGui::EndTabItem(); }

    bool initialized = false;
    bool displayed = false;
    bool waitForMenuKeyRelease = false;

    HGLRC imguiWglContext;
    HWND hWnd_during_init;

    bool cg_fovScaleEnable = false;
    float cg_fovScale = 0.f;

    static void toggle_menu_cmd()
    {
        toggle_menu(false);
    }

    void toggle_menu(bool closedUsingButton)
    {
        if (closedUsingButton)
        {
            *stock::mouseInitialized = stock::qtrue;
            stock::IN_ActivateMouse();
            return;
        }
        
        if (!displayed)
        {
            displayed = true;
            stock::IN_DeactivateMouse();
            *stock::mouseActive = stock::qfalse;
            *stock::mouseInitialized = stock::qfalse;
        }
        else
        {
            displayed = false;
            *stock::mouseInitialized = stock::qtrue;
            stock::IN_ActivateMouse();
        }
    }

    static void init(HDC hdc)
    {
        hWnd_during_init = *stock::hWnd;
        imguiWglContext = wglCreateContext(hdc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "iw1x_imgui.ini";
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 16.0f);

        ImGui_ImplWin32_InitForOpenGL(*stock::hWnd);
        ImGui_ImplOpenGL2_Init();

        initialized = true;
    }

    static void gui_on_frame()
    {
        new_frame();
        draw_menu();
        end_frame();
    }
    
    void new_frame()
    {
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }
    
    static void menu_loads_settings()
    {
        cg_fovScaleEnable = view::cg_fovScaleEnable->integer;
        cg_fovScale = view::cg_fovScale->value;
    }
    
    void draw_menu()
    {
        menu_loads_settings();

        ImGui::SetNextWindowSize(ImVec2(300, 0));
        ImGui::SetNextWindowPos(ImVec2(25, 80), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowFocus();
        ImGui::Begin("##", &displayed, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        if (ImGui::BeginTabBar("TabBar"))
        {
            draw_menu_tab_View();

            ImGui::EndTabBar();
        }
        
        ImGui::End();
        
        if (!displayed)
            toggle_menu(true);
        
        menu_updates_settings();
    }

    void draw_menu_tab_View()
    {
        BEGINTABITEM_SPACE("View")
            
            ImGui::Checkbox("FOV scale", &cg_fovScaleEnable);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (!cg_fovScaleEnable) ImGui::BeginDisabled();
            ImGui::SliderFloat("##slider_cg_fovScale", &cg_fovScale, view::cg_fovScale_min, view::cg_fovScale_max, "%.2f", ImGuiSliderFlags_NoInput);
            if (!cg_fovScaleEnable) ImGui::EndDisabled();

        ENDTABITEM_SPACED()
    }

    void menu_updates_settings()
    {
        stock::Cvar_Set(view::cg_fovScaleEnable->name, cg_fovScaleEnable ? "1" : "0");
        stock::Cvar_Set(view::cg_fovScale->name, utils::string::va("%.2f", cg_fovScale));
    }

    void end_frame()
    {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }
    
    static void shutdown()
    {
        if (initialized)
        {
            ImGui_ImplOpenGL2_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            initialized = false;
        }
    }
    
    static BOOL WINAPI stub_SwapBuffers(HDC hdc)
    {
        if (!initialized)
        {
            init(hdc);
        }
        else
        {
            if (WindowFromDC(hdc) != hWnd_during_init) // Caused by Alt+Enter / vid_restart
            {
                ImGui_ImplOpenGL2_Shutdown();
                ImGui_ImplWin32_Shutdown();
                ImGui::DestroyContext();
                init(hdc);
            }
        }

        if (displayed)
        {
            auto originalWglContext = wglGetCurrentContext();

            wglMakeCurrent(hdc, imguiWglContext);
            gui_on_frame();
            wglMakeCurrent(hdc, originalWglContext);
        }
        
        return SwapBuffers(hdc);
    }
    
    class component final : public component_interface
    {
    public:
        void* load_import(const std::string&, const std::string& function) override
        {
            if (function == "SwapBuffers")
                return stub_SwapBuffers;
            return nullptr;
        }

        void post_unpack() override
        {
            stock::Cmd_AddCommand("imgui", toggle_menu_cmd);
        }
        
        void pre_destroy() override
        {
            shutdown();
        }
    };
}

REGISTER_COMPONENT(imgui::component)
#endif
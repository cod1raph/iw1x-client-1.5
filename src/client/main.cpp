#include "pch.h"

#include "hook.h"
#include "string.h"
#include "io.h"

#include "loader/loader.h"
#include "loader/component_loader.h"

bool clientNamedMohaa = false;
DWORD address_cgame_mp;
DWORD address_ui_mp;
utils::hook::detour hook_GetModuleFileNameW;
utils::hook::detour hook_GetModuleFileNameA;

static void MSG_ERR(const char* msg)
{
    MessageBox(NULL, msg, MOD_NAME, MB_ICONERROR | MB_SETFOREGROUND);
}

static LONG WINAPI CrashLogger(EXCEPTION_POINTERS* exceptionPointers)
{
    std::string crashFilename = std::string(MOD_NAME) + "_crash.log";
    std::ofstream logFile(crashFilename);
    if (logFile.is_open())
    {
        HANDLE hProcess = GetCurrentProcess();
        SymInitialize(hProcess, nullptr, TRUE);

        auto exceptionAddress = exceptionPointers->ExceptionRecord->ExceptionAddress;
        auto exceptionCode = exceptionPointers->ExceptionRecord->ExceptionCode;

        IMAGEHLP_MODULE moduleInfo = { sizeof(moduleInfo) };
        SymGetModuleInfo(hProcess, reinterpret_cast<DWORD>(exceptionAddress), &moduleInfo);
        std::filesystem::path loadedImageName = moduleInfo.LoadedImageName;
        auto file = loadedImageName.filename().string();

        logFile << "File: " << file << std::endl;
        logFile << "Exception Address: 0x" << std::hex << exceptionAddress << std::endl;
        logFile << "Exception Code: 0x" << std::hex << exceptionCode << std::endl;

        std::string errorMessage = "A crash occured, " + crashFilename + " available in your CoD folder.";

        MSG_ERR(errorMessage.c_str());
    }
    return EXCEPTION_EXECUTE_HANDLER;
}













static FARPROC WINAPI stub_GetProcAddress(const HMODULE hModule, const LPCSTR lpProcName)
{
    if (HIWORD(lpProcName) == 0)
    {
        return GetProcAddress(hModule, lpProcName);
    }

#if 0
    std::stringstream ss;
    ss << "###### stub_GetProcAddress: lpProcName: " << lpProcName << std::endl;
    OutputDebugString(ss.str().c_str());
#endif
    

        
    if (strstr(lpProcName, "Steam"))
    {
        std::stringstream ss;
        ss << "###### stub_GetProcAddress strstr Steam: " << lpProcName << std::endl;
        OutputDebugString(ss.str().c_str());
    }



    //if (!strcmp(lpProcName, "GlobalMemoryStatus"))
        //component_loader::post_unpack();


    return GetProcAddress(hModule, lpProcName);
}

static HMODULE WINAPI stub_LoadLibraryA(LPCSTR lpLibFileName)
{
    auto ret = LoadLibraryA(lpLibFileName);
    auto hModule_address = (DWORD)GetModuleHandleA(lpLibFileName);

    if (lpLibFileName != NULL)
    {
        auto fileName = PathFindFileNameA(lpLibFileName);

#if 0
        std::stringstream ss;
        ss << "###### stub_LoadLibraryA: fileName: " << fileName << std::endl;
        OutputDebugString(ss.str().c_str());
#endif


        if (!strcmp(fileName, "steam.dll"))
        {


        }



    }
    return ret;
}

/*
Return original client filename, so GPU driver knows what game it is,
so if it has a profile for it, it will get enabled
(this prevents buffer overrun when glGetString(GL_EXTENSIONS) gets called)
*/
// For AMD and Intel HD Graphics
static DWORD WINAPI stub_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    auto* orig = static_cast<decltype(GetModuleFileNameA)*>(hook_GetModuleFileNameA.get_original());
    auto ret = orig(hModule, lpFilename, nSize);
    
    if (!strcmp(PathFindFileNameA(lpFilename), "iw1x-1.5.exe"))
    {
        std::filesystem::path path = lpFilename;
        auto binary = "CoDMP.exe";
        path.replace_filename(binary);
        std::string pathStr = path.string();
        std::copy(pathStr.begin(), pathStr.end(), lpFilename);
        lpFilename[pathStr.size()] = '\0';
    }
    return ret;
}
// For Nvidia
static DWORD WINAPI stub_GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
    auto* orig = static_cast<decltype(GetModuleFileNameW)*>(hook_GetModuleFileNameW.get_original());
    auto ret = orig(hModule, lpFilename, nSize);

    int required_size = WideCharToMultiByte(CP_UTF8, 0, lpFilename, -1, nullptr, 0, nullptr, nullptr);
    std::string pathStr(required_size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, lpFilename, -1, pathStr.data(), required_size, nullptr, nullptr);

    if (!strcmp(PathFindFileNameA(pathStr.c_str()), "iw1x-1.5.exe"))
    {
        std::filesystem::path pathFs = pathStr;

        auto client_filename = "CoDMP.exe";
        pathFs.replace_filename(client_filename);
        pathStr = pathFs.string();

        required_size = MultiByteToWideChar(CP_UTF8, 0, pathStr.c_str(), -1, nullptr, 0);
        MultiByteToWideChar(CP_UTF8, 0, pathStr.c_str(), -1, lpFilename, required_size);
    }
    return ret;
}

static bool compare_md5(const std::string& data, const std::string& expected_hash)
{
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    BYTE hash[16];
    DWORD hashSize = sizeof(hash);
    char hex_hash[33]{};
    
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return false;
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    { 
        CryptReleaseContext(hProv, 0);
        return false;
    }
    if (!CryptHashData(hHash, (BYTE*)data.data(), data.size(), 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashSize, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    
    for (int i = 0; i < 16; i++)
        sprintf_s(hex_hash + i * 2, 3, "%02X", hash[i]);
    hex_hash[32] = '\0';
    
    return expected_hash == hex_hash;
}

static bool read_file(const std::string& file, std::string* data)
{
    if (!data)
        return false;
    data->clear();

    if (std::ifstream(file).good())
    {
        std::ifstream stream(file, std::ios::binary);
        if (!stream.is_open())
            return false;

        stream.seekg(0, std::ios::end);
        const std::streamsize size = stream.tellg();
        stream.seekg(0, std::ios::beg);

        if (size > -1)
        {
            data->resize(static_cast<uint32_t>(size));
            stream.read(data->data(), size);
            stream.close();
            return true;
        }
    }

    return false;
}

static FARPROC load_binary()
{
    loader loader;
    const utils::nt::library self;
    
    loader.set_import_resolver([self](const std::string& library, const std::string& function) -> void*
        {
#if 1
            std::stringstream ss;
            ss << "###### set_import_resolver: library: " << library << ", function: " << function << std::endl;
            OutputDebugString(ss.str().c_str());
#endif
            
            
            if (function == "GetProcAddress")
                return stub_GetProcAddress;
            if (function == "LoadLibraryA")
                return stub_LoadLibraryA;

            return component_loader::load_import(library, function);
        });

    const utils::nt::library kernel32("kernel32.dll");
    hook_GetModuleFileNameW.create(kernel32.get_proc<DWORD(WINAPI*)(HMODULE, LPWSTR, DWORD)>("GetModuleFileNameW"), stub_GetModuleFileNameW);
    hook_GetModuleFileNameA.create(kernel32.get_proc<DWORD(WINAPI*)(HMODULE, LPSTR, DWORD)>("GetModuleFileNameA"), stub_GetModuleFileNameA);

    auto client_filename = "CoDMP.exe";

    std::string data_codmp;

    if (!read_file(client_filename, &data_codmp))
    {
        std::stringstream ss;
        ss << "Failed to read " << client_filename;
        ss << std::endl << std::endl << "Is " << MOD_NAME << " in your CoD folder?";
        throw std::runtime_error(ss.str());
    }
    
    if (!compare_md5(data_codmp, "4F4596B1CDB21F9EB62E6683ECF48DC6"))
    {
        std::stringstream ss;
        ss << "Your " << client_filename << " file hash doesn't match the original.";
        throw std::runtime_error(ss.str());
    }
    
    return loader.load(self, data_codmp);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int)
{
#if 0
    MessageBox(NULL, lpCmdLine, "", NULL);
#endif
    
    SetUnhandledExceptionFilter(CrashLogger);
#if 0
    // Crash test
    * (int*)nullptr = 1;
#endif

#ifdef DEBUG
    // Delete stock crash file
    DeleteFileA("__codmp");
    DeleteFileA("__mohaa");
#endif
    
    auto premature_shutdown = true;
    const auto _ = gsl::finally([&premature_shutdown]()
        {
            if (premature_shutdown)
                component_loader::pre_destroy();
        });

    FARPROC entry_point;
    try
    {
        if (!component_loader::post_start())
            return 1;

        entry_point = load_binary();
        if (!entry_point)
            throw std::runtime_error("Unable to load binary into memory");
        
        if (!component_loader::post_load())
            return 1;
    }
    catch (const std::exception& ex)
    {
        MSG_ERR(ex.what());
        return 1;
    }
    
    return static_cast<int>(entry_point());
}
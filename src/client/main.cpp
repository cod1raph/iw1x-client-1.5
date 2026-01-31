#include "pch.h"

#include "hook.h"
#include "string.h"
#include "io.h"

#include "loader/loader.h"
#include "loader/component_loader.h"



DWORD address_cgame_mp;
DWORD address_ui_mp;
utils::hook::detour hook_GetModuleFileNameW;
utils::hook::detour hook_GetModuleFileNameA;


#if 0
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

    if (!strcmp(lpProcName, "SteamIsAppSubscribed"))
    {

    }
    if (!strcmp(lpProcName, "SteamStartup"))
    {

    }
    if (!strcmp(lpProcName, "SteamCleanup"))
    {

    }




    //if (!strcmp(lpProcName, "GlobalMemoryStatus"))
        //component_loader::post_unpack();

    return GetProcAddress(hModule, lpProcName);
}
#endif

static HMODULE WINAPI stub_LoadLibraryA(LPCSTR lpLibFileName)
{
    auto ret = LoadLibraryA(lpLibFileName);
    if (lpLibFileName != NULL)
    {
#if 0
        std::stringstream ss;
        ss << "###### stub_LoadLibraryA: lpLibFileName: " << lpLibFileName << std::endl;
        OutputDebugString(ss.str().c_str());
#endif



        if (!strcmp(lpLibFileName, "steam.dll"))
        {
            MessageBox(NULL, "will now lose hooks", MOD_NAME, MB_ICONINFORMATION | MB_SETFOREGROUND);
        }


        if (!strcmp(lpLibFileName, "ui_mp_x86.dll"))
        {
            MessageBox(NULL, "WORKS", MOD_NAME, MB_ICONINFORMATION | MB_SETFOREGROUND); // Never reached
        }




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
#if 0
            std::stringstream ss;
            ss << "###### set_import_resolver: library: " << library << ", function: " << function << std::endl;
            OutputDebugString(ss.str().c_str());
#endif
            //if (function == "GetProcAddress")
                //return stub_GetProcAddress;
            if (function == "LoadLibraryA")
                return stub_LoadLibraryA;

            return component_loader::load_import(library, function);
        });

    auto client_filename = "CoDMP_o.exe";
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

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, LPSTR, _In_ int)
{
#ifdef DEBUG
    // Delete stock crash file
    DeleteFileA("__codmp");
#endif

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
        MessageBox(NULL, ex.what(), MOD_NAME, MB_ICONERROR | MB_SETFOREGROUND);
        return 1;
    }
    
    return static_cast<int>(entry_point());
}
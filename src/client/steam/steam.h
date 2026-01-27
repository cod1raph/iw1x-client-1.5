#pragma once

#include "nt.h"

#define STEAM_EXPORT extern "C" __declspec(dllexport)

#include "interfaces/apps.h"

#include <map>

namespace steam
{


    class callbacks
    {
    public:
        class base
        {
        public:
            base() : flags_(0), callback_(0)
            {
            }

            virtual void run(void* pv_param) = 0;
            virtual void run(void* pv_param, bool failure, uint64_t handle) = 0;
            virtual int get_callback_size_bytes() = 0;

            int get_i_callback() const { return callback_; }
            void set_i_callback(const int i_callback) { callback_ = i_callback; }

        protected:
            ~base() = default;

            unsigned char flags_;
            int callback_;
        };

        struct result final
        {
            void* data{};
            int size{};
            int type{};
            uint64_t call{};
        };

        static uint64_t register_call();

        static void register_callback(base* handler, int callback);
        static void unregister_callback(base* handler);

        static void register_call_result(uint64_t call, base* result);
        static void unregister_call_result(uint64_t call, base* result);

        static void return_call(void* data, int size, int type, uint64_t call);
        static void run_callbacks();

    private:
        static uint64_t call_id_;
        static std::recursive_mutex mutex_;
        static std::map<uint64_t, bool> calls_;
        static std::map<uint64_t, base*> result_handlers_;
        static std::vector<result> results_;
        static std::vector<base*> callback_list_;
    };



    STEAM_EXPORT bool SteamAPI_Init();
    STEAM_EXPORT const char* SteamAPI_GetSteamInstallPath();
}